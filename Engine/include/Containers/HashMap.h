#pragma once

#include "Config.h"

#include "System/Memory.h"

#include "Math/Math.h"

#include "Containers/Hash.h"
#include "Containers/Pair.h"
#include "Containers/TemplateUtil.h"
#include "Common/Types.h"

#include <utility>
#include <emmintrin.h>
#include <immintrin.h>

namespace hs
{

//------------------------------------------------------------------------------
template<class T>
struct DefaultEquals
{
    using TypeT = typename ArgumentTypeTempl<T>::Type;
    static bool Equals(TypeT a, TypeT b)
    {
        return a == b;
    }
};

//------------------------------------------------------------------------------
struct HashMapConstants
{
    static constexpr    Hash_t  VALID_ELEMENT_MASK = 1 << 7;    // 0b1000_0000
    static constexpr    uint8   TOMBSTONE_MASK = 1 << 6;        // 0b0100_0000
    static constexpr    uint8   LOW_MASK = 0x7F;                // 0b0111_1111
    static constexpr    uint8   LOW_SHIFT = 8;
    static constexpr    float   MAX_LOAD_FACTOR = 0.8f;
    static constexpr    uint8   MIN_EXPONENT = 4;
    static constexpr    Index_t NPOS = -1;
    static const        __m128i EMPTY_MASK_128;
};

//------------------------------------------------------------------------------
template<class K, class V, class HashF = DefaultHash<K>, class EqualsF = DefaultEquals<K>>
class HashMap
{
    enum class FindItemMode;

public:
    class Entry;
    using KeyArg_t = typename ArgumentTypeTempl<K>::Type;
    using ValueArg_t = typename ArgumentTypeTempl<V>::Type;
    using Item_t = Pair<const K, V>;

    template<class HashMapT, class ItemT>
    class IteratorTempl;
    using Iterator_t = IteratorTempl<HashMap, Item_t>;
    using ConstIterator_t = IteratorTempl<const HashMap, const Item_t>;

    //-----------------------------------------------------------------------------
    HashMap()
    {
        static_assert((1 << HashMapConstants::MIN_EXPONENT) >= sizeof(__m128i), "We use SSE and metadata size don't use ceil");

        AllocArrays();
    }

    // TODO operators

    //-----------------------------------------------------------------------------
    ~HashMap()
    {
        for (Index_t i = 0; i < capacity_; ++i)
        {
            if (metadata_[i] & HashMapConstants::VALID_ELEMENT_MASK)
            {
                items_[i].~Item_t();
            }
        }

        FreeAligned(metadata_);
        FreeAligned(items_);
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] constexpr Index_t Count() const
    {
        return count_;
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] constexpr Index_t Capacity() const
    {
        return capacity_;
    }

    //-----------------------------------------------------------------------------
    [[nodiscard]] constexpr float LoadFactor() const
    {
        return static_cast<float>(count_) / capacity_;
    }

    //------------------------------------------------------------------------------
    bool Insert(KeyArg_t key, ValueArg_t value)
    {
        Pair<bool, V&> item = FindOrEmplace(key, value);
        return item.first;
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] bool Contains(KeyArg_t key) const
    {
        Pair<bool, Item_t*> item = FindItem<FindItemMode::Find>(key);
        return item.first;
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] Iterator_t Find(KeyArg_t key)
    {
        Pair<bool, Item_t*> item = FindItem<FindItemMode::Find>(key);
        if (!item.first)
            return end();

        return Iterator_t{ this, static_cast<Index_t>(item.second - items_) };
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] ConstIterator_t Find(KeyArg_t key) const
    {
        Pair<bool, Item_t*> item = FindItem<FindItemMode::Find>(key);
        if (!item.first)
            return cend();

        return ConstIterator_t{ this, static_cast<Index_t>(item.second - items_) };
    }

    //------------------------------------------------------------------------------
    /*!
    Tires to find a value with a given key or creates a new entry.
    \return bool - is the value found? V& ref to the found or newly created value.
    */
    template<class... ArgsT>
    [[nodiscard]] Pair<bool, V&> FindOrEmplace(KeyArg_t key, ArgsT&&... args)
    {
        Pair<bool, Item_t*> item = FindItem<FindItemMode::Insert>(key);
        if (item.first)
        {
            return { true, item.second->Value() };
        }
        else
        {
            // The metadata is already modified by the FindItem -> we must place the item before rehash
            ++count_;
            new (item.second) Item_t (key, V(std::forward<ArgsT>(args)...));

            if (LoadFactor() > HashMapConstants::MAX_LOAD_FACTOR)
            {
                Rehash();
                // We do the find here again, but rehash is already an expensive operation
                // that should happen rarely
                item = FindItem<FindItemMode::Find>(key);
                HS_ASSERT(item.first);
            }

            HS_ASSERT(count_ < capacity_ && "There has to always be at least one empty spot so we can always insert");
            return { false, item.second->Value() };
        }
    }

    //------------------------------------------------------------------------------
    /*!
    Tires to find a value with a given key or creates a new entry.
    \return bool - is the value found? V& ref to the found or newly created value.
    */
    [[nodiscard]] Pair<bool, V&> FindOrInsert(KeyArg_t key, ValueArg_t value)
    {
        return FindOrEmplace(key, value);
    }

    //------------------------------------------------------------------------------
    /*!
    Tires to find a value with a given key or creates a new entry with a default
    constructed value.
    \return bool - is the value found? V& ref to the found or newly created value.
    */
    [[nodiscard]] Pair<bool, V&> FindOrDefault(KeyArg_t key)
    {
        return FindOrEmplace(key);
    }

    //------------------------------------------------------------------------------
    /*!
    Returns Entry which can be used to create the value in the hashmap later
    */
    [[nodiscard]] Entry GetEntry(KeyArg_t key)
    {
        Pair<bool, Item_t*> item = FindItem<FindItemMode::Insert>(key);
        const bool found = item.first;

        if (!found)
        {
            ++count_;
            // NOTE(pavel): This may be undefined behavior to assume Item_t is created
            // and to placement new to a subobject of it... but who knows
            new ((uint8*)item.second + offsetof(Item_t, first)) K(key);

            if (LoadFactor() > HashMapConstants::MAX_LOAD_FACTOR)
            {
                Rehash();
                // We do the find here again, but rehash is already an expensive operation
                // that should happen rarely
                item = FindItem<FindItemMode::Find>(key);
                HS_ASSERT(item.first);
            }
            HS_ASSERT(count_ < capacity_ && "There has to always be at least one empty spot so we can always insert");
        }

        return Entry((V*)((uint8*)item.second + offsetof(Item_t, second)), found);
    }

    //------------------------------------------------------------------------------
    void Remove(KeyArg_t key)
    {
        Pair<bool, Item_t*> result = FindItem<FindItemMode::Remove>(key);
        if (!result.first)
            return;

        result.second->~Item_t();
        --count_;
    }

private:
    Item_t* items_;
    union
    {
        uint8_t* metadata_;
        __m128i* metadata_m128_;
    };

    Index_t count_{};
    Index_t capacity_{ static_cast<Index_t>(1) << HashMapConstants::MIN_EXPONENT };

    //------------------------------------------------------------------------------
    void AllocArrays()
    {
        items_ = reinterpret_cast<Item_t*>(AllocAligned(capacity_ * sizeof(Item_t), alignof(Item_t)));
        const auto metadataSize = capacity_; // One byte per hashmap item
        metadata_m128_ = reinterpret_cast<__m128i*>(AllocAligned(metadataSize, alignof(__m128i)));
        memset(metadata_m128_, 0, metadataSize);
    }

    //-----------------------------------------------------------------------------
    [[nodiscard]] constexpr Hash_t ComputeHashHigh(Hash_t hash) const
    {
        return hash >> HashMapConstants::LOW_SHIFT;
    }

    //-----------------------------------------------------------------------------
    [[nodiscard]] constexpr uint8_t ComputeHashLow(Hash_t hash) const
    {
        return static_cast<uint8_t>(hash & HashMapConstants::LOW_MASK);
    }

    //-----------------------------------------------------------------------------
    enum class FindItemMode
    {
        Find,       // Do not modify the metedata, only try to find the item
        Insert,     // If not present, prepare a slot for the item insertion
        InsertFast, // The item cannot be in the map, only search for a slot where to insert
        Remove      // If found, modify the metadata, otherwise do nothing
    };

    //-----------------------------------------------------------------------------
    // return   bool - is entry found?
    //          Item_t* - if insert is true, we can write here
    template<FindItemMode mode>
    [[nodiscard]] Pair<bool, Item_t*> FindItem(KeyArg_t key) const
    {
        const Hash_t hash = HashF::Hash(key);
        const uint8_t hashLow = ComputeHashLow(hash);
        const Hash_t hashHigh = ComputeHashHigh(hash);
        const Hash_t modMask = capacity_ - 1;
        const Hash_t startIndex = static_cast<int>(hashHigh & modMask);

        for (int i = startIndex;;)
        {
            // items_[i] is empty - this is the spot
            if ((metadata_[i] & HashMapConstants::VALID_ELEMENT_MASK) == 0)
            {
                if constexpr (mode == FindItemMode::Insert || mode == FindItemMode::InsertFast)
                {
                    metadata_[i] = hashLow | HashMapConstants::VALID_ELEMENT_MASK;
                    return { false, items_ + i };
                }
                else
                {
                    return { false, nullptr };
                }
            }

            if constexpr (mode != FindItemMode::InsertFast)
            {
                // if key already present, disallow second insertion
                if ((metadata_[i] & HashMapConstants::LOW_MASK) == hashLow && EqualsF::Equals(items_[i].Key(), key))
                {
                    // Item found
                    if constexpr (mode == FindItemMode::Remove)
                    {
                        metadata_[i] = HashMapConstants::TOMBSTONE_MASK;
                    }
                    return { true, items_ + i };
                }
            }

            i = (i + 1) & modMask;
            // Wrap around is not possible - it would mean that the table is 100% full
        }

        HS_ASSERT(!"Impossible");
        return { false, nullptr };
    }

    //------------------------------------------------------------------------------
    void Rehash()
    {
        const Hash_t oldCapacity = capacity_;
        capacity_ = capacity_ << 1;
        HS_ASSERT(oldCapacity < capacity_);

        auto oldItems = items_;
        auto oldMetadata = metadata_;

        AllocArrays();

        for (Index_t i = 0; i < oldCapacity; ++i)
        {
            if (oldMetadata[i] & HashMapConstants::VALID_ELEMENT_MASK)
            {
                // This insert may be optimized - duplicate keys do not have to be checked
                Pair<bool, Item_t*> insertSpot = FindItem<FindItemMode::InsertFast>(oldItems[i].Key());
                HS_ASSERT(!insertSpot.first);
                new (insertSpot.second) Item_t(oldItems[i]);
                oldItems[i].~Item_t();
            }
        }

        FreeAligned(oldItems);
        FreeAligned(oldMetadata);
    }

    //------------------------------------------------------------------------------
    template<class IteratorT>
    void Next(IteratorT& it) const
    {
        do
        {
            ++it.index_;
            if (metadata_[it.index_] & HashMapConstants::VALID_ELEMENT_MASK)
                break;
        } while (it.index_ < capacity_);
    }

    //------------------------------------------------------------------------------
    template<class IteratorT>
    void Previous(IteratorT& it) const
    {
        do
        {
            --it.index_;
            if (metadata_[it.index_] & HashMapConstants::VALID_ELEMENT_MASK)
                break;
        } while (it.index_ >= 0);
    }

    //------------------------------------------------------------------------------
    Item_t* At(Index_t index)
    {
        return items_ + index;
    }

    //------------------------------------------------------------------------------
    const Item_t* At(Index_t index) const
    {
        return items_ + index;
    }

    //------------------------------------------------------------------------------
    ConstIterator_t FindBegin() const
    {
        Index_t index = 0;
        do
        {
            if (metadata_[index] & HashMapConstants::VALID_ELEMENT_MASK)
                break;
            ++index;
        } while (index < capacity_);

        return ConstIterator_t(this, index);
    }

    //------------------------------------------------------------------------------
    Iterator_t FindBegin()
    {
        Index_t index = 0;
        do
        {
            if (metadata_[index] & HashMapConstants::VALID_ELEMENT_MASK)
                break;
            ++index;
        } while (index < capacity_);

        return Iterator_t(this, index);
    }

public:
    //------------------------------------------------------------------------------
    /*!
    Move-only to be able to consistently check whether every Entry we get from a HashMap
    is initialized before it is destroyed.
    It's not a 100% protection since somebody can keep an Entry around, manipulate (rehash)
    the HashMap and then try to use the Entry, however, it's better than nothing.
    */
    class Entry
    {
    public:
        //------------------------------------------------------------------------------
        bool IsInitialized() const
        {
            return isInitialized_;
        }

        //------------------------------------------------------------------------------
        template<class... ArgsT>
        void Construct(ArgsT&&... args)
        {
            HS_ASSERT(!isInitialized_);
            new (value_) V(std::forward<ArgsT>(args)...);
        }

        //------------------------------------------------------------------------------
        [[nodiscard]] V& Get()
        {
            HS_ASSERT(isInitialized_);
            return *value;
        }

        //------------------------------------------------------------------------------
        Entry(const Entry&) = delete;
        Entry& operator=(const Entry&) = delete;

        //------------------------------------------------------------------------------
        Entry(Entry&& other)
        {
            isInitialized_ = other.isInitialized_;
            value_ = other.value_;

            other.isInitialized_ = true;
            other.value_ = nullptr;
        }

        //------------------------------------------------------------------------------
        Entry& operator=(Entry&& other)
        {
            // Tmp vars for branchless self-assingment protection
            auto isInitializedTmp = other.isInitialized_;
            auto valueTmp = other.value_;

            other.isInitialized_ = true;
            other.value_ = nullptr;

            isInitialized_ = isInitializedTmp;
            value_ = valueTmp;
        }

        //------------------------------------------------------------------------------
        ~Entry()
        {
            HS_ASSERT(isInitialized_ && "Entry in the hashmap needs to be initialized unless the hasmap in left in an invalid state");
        }

    private:
        V* value_; //!< Either found object or just a place in memory where we can placement new the object
        bool isInitialized_;

        //------------------------------------------------------------------------------
        Entry(V* value, bool isInitialized) : value_(value), isInitialized_(isInitialized) {}
    };

    //------------------------------------------------------------------------------
    // Iterators
    //------------------------------------------------------------------------------
    #pragma region Iterators
    template<class HashMapT, class ItemT>
    class IteratorTempl
    {
        friend HashMapT;
    public:
        //------------------------------------------------------------------------------
        ItemT& operator*()
        {
            return *hashMap_->At(index_);
        }

        //------------------------------------------------------------------------------
        const ItemT& operator*() const
        {
            return *hashMap_->At(index_);
        }

        //------------------------------------------------------------------------------
        ItemT* operator->()
        {
            return hashMap_->At(index_);
        }

        //------------------------------------------------------------------------------
        const ItemT* operator->() const
        {
            return hashMap_->At(index_);
        }

        //------------------------------------------------------------------------------
        bool operator==(const IteratorTempl& other) const
        {
            return index_ == other.index_;
        }

        //------------------------------------------------------------------------------
        bool operator!=(const IteratorTempl& other) const
        {
            return !(*this == other);
        }

        //------------------------------------------------------------------------------
        IteratorTempl& operator++()
        {
            hashMap_->Next(*this);
            return *this;
        }

        //------------------------------------------------------------------------------
        IteratorTempl operator++(int)
        {
            IteratorTempl tmp = *this;
            ++(*this);
            return tmp;
        }

        //------------------------------------------------------------------------------
        IteratorTempl& operator--()
        {
            hashMap_->Previous(*this);
            return *this;
        }

        //------------------------------------------------------------------------------
        IteratorTempl operator--(int)
        {
            IteratorTempl tmp = *this;
            --(*this);
            return tmp;
        }

    private:
        HashMapT* hashMap_;
        Index_t index_;

        IteratorTempl(HashMapT* hashMap, Index_t index) : hashMap_(hashMap), index_(index) {}
    };

    //------------------------------------------------------------------------------
    [[nodiscard]] ConstIterator_t cbegin() const
    {
        return FindBegin();
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] ConstIterator_t begin() const
    {
        return cbegin();
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] Iterator_t begin()
    {
        return FindBegin();
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] ConstIterator_t cend() const
    {
        return ConstIterator_t(this, capacity_);
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] ConstIterator_t end() const
    {
        return cend();
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] Iterator_t end()
    {
        return Iterator_t(this, capacity_);
    }
    #pragma endregion
};

}
