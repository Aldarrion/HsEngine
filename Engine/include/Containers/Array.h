#pragma once

#include "Config.h"

#include "System/Memory.h"

#include "Containers/Span.h"

#include "Math/Math.h"
#include "Common/Types.h"
#include "Common/Assert.h"

#include <cstring>
#include <cstddef>
#include <type_traits>
#include <initializer_list>
#include <new>

namespace hs
{

//------------------------------------------------------------------------------
template<class T, Index_t capacity_>
class StaticMemoryPolicy
{
public:
    static constexpr bool IS_MOVABLE = false;

    //------------------------------------------------------------------------------
    void CopyConstruct(const StaticMemoryPolicy<T, capacity_>& other)
    {
        count_ = other.count_;
    }

    //------------------------------------------------------------------------------
    void Assign(const StaticMemoryPolicy<T, capacity_>& other)
    {
        count_ = other.count_;
    }

    //------------------------------------------------------------------------------
    void Free()
    {
        count_ = 0;
    }

    //------------------------------------------------------------------------------
    void EnsureEmplaceBack()
    {
        HS_ASSERT(false && "Static array size cannot grow");
    }

    //------------------------------------------------------------------------------
    void EnsureEmplace(Index_t index)
    {
        HS_ASSERT(false && "Static array size cannot grow");
    }

    //------------------------------------------------------------------------------
    void Grow(Index_t capacity)
    {
        HS_ASSERT(false && "Static array size cannot grow");
    }

    //------------------------------------------------------------------------------
    Index_t& CountMut()
    {
        return count_;
    }

    //------------------------------------------------------------------------------
    Index_t Count() const
    {
        return count_;
    }

    //------------------------------------------------------------------------------
    T* Items()
    {
        return reinterpret_cast<T*>(items_);
    }

    //------------------------------------------------------------------------------
    const T* Items() const
    {
        return reinterpret_cast<const T*>(items_);
    }

    //------------------------------------------------------------------------------
    Index_t Capacity() const
    {
        return capacity_;
    }

private:
    alignas(T) uint8 items_[capacity_ * sizeof(T)];
    Index_t count_{};
};

//------------------------------------------------------------------------------
template<class T>
class GrowableMemoryPolicy
{
public:
    static constexpr bool IS_MOVABLE = true;

    //------------------------------------------------------------------------------
    void CopyConstruct(const GrowableMemoryPolicy<T>& other)
    {
        HS_ASSERT(!items_);

        capacity_ = other.capacity_;
        count_ = other.count_;
        items_ = static_cast<T*>(AllocAligned(sizeof(T) * capacity_, alignof(T)));
    }

    //------------------------------------------------------------------------------
    void Assign(const GrowableMemoryPolicy<T>& other)
    {
        if (capacity_ != other.capacity_)
        {
            FreeAligned(items_);
            items_ = static_cast<T*>(AllocAligned(sizeof(T) * other.capacity_, alignof(T)));
        }

        capacity_ = other.capacity_;
        count_ = other.count_;
    }

    //------------------------------------------------------------------------------
    void MoveConstruct(GrowableMemoryPolicy<T>&& other)
    {
        capacity_ = other.capacity_;
        count_ = other.count_;
        items_ = other.items_;

        other.items_ = nullptr;
        other.capacity_ = 0;
        other.count_ = 0;
    }

    //------------------------------------------------------------------------------
    void MoveAssign(GrowableMemoryPolicy<T>&& other)
    {
        FreeAligned(items_);
        capacity_ = other.capacity_;
        count_ = other.count_;
        items_ = other.items_;

        other.items_ = nullptr;
        other.capacity_ = 0;
        other.count_ = 0;
    }

    //------------------------------------------------------------------------------
    void Free()
    {
        count_ = 0;
        capacity_ = 0;
        FreeAligned(items_);
    }

    //------------------------------------------------------------------------------
    void EnsureEmplaceBack()
    {
        Grow(GetNextCapacity());
    }

    //------------------------------------------------------------------------------
    void EnsureEmplace(Index_t index)
    {
        capacity_ = GetNextCapacity();

        auto newItems = static_cast<T*>(AllocAligned(sizeof(T) * capacity_, alignof(T)));
        if constexpr (std::is_trivial_v<T>)
        {
            memcpy(newItems, items_, sizeof(T) * index);
            memcpy(&newItems[index + 1], &items_[index], (count_ - index) * sizeof(T));
        }
        else
        {
            for (Index_t i = 0; i < index; ++i)
            {
                new(newItems + i) T(std::move(items_[i]));
                items_[i].~T();
            }
            for (Index_t i = index; i < count_; ++i)
            {
                new(newItems + i + 1) T(std::move(items_[i]));
                items_[i].~T();
            }
        }
        FreeAligned(items_);
        items_ = newItems;
    }

    //------------------------------------------------------------------------------
    void Grow(Index_t capacity)
    {
        capacity_ = Max(capacity, MIN_CAPACITY);
        auto newItems = static_cast<T*>(AllocAligned(sizeof(T) * capacity_, alignof(T)));
        if constexpr (std::is_trivial_v<T>)
        {
            memcpy(newItems, items_, sizeof(T) * count_);
        }
        else
        {
            for (Index_t i = 0; i < count_; ++i)
            {
                new(newItems + i) T(std::move(items_[i]));
                items_[i].~T();
            }
        }

        FreeAligned(items_);
        items_ = newItems;
    }

    //------------------------------------------------------------------------------
    Index_t& CountMut()
    {
        return count_;
    }

    //------------------------------------------------------------------------------
    Index_t Count() const
    {
        return count_;
    }

    //------------------------------------------------------------------------------
    T* Items()
    {
        return items_;
    }

    //------------------------------------------------------------------------------
    const T* Items() const
    {
        return items_;
    }

    //------------------------------------------------------------------------------
    Index_t Capacity() const
    {
        return capacity_;
    }

private:
    static constexpr Index_t MIN_CAPACITY = 8;

    Index_t capacity_{};
    Index_t count_{};
    T* items_{};

    //------------------------------------------------------------------------------
    Index_t GetNextCapacity() const
    {
        Index_t newCapacity = Max(NextPow2(static_cast<uint>(capacity_)), static_cast<uint>(MIN_CAPACITY));
        HS_ASSERT(newCapacity > 0);
        return newCapacity;
    }
};

//------------------------------------------------------------------------------
template<class T, class MemoryPolicy>
class TemplArray
{
public:
    using Item_t = T;
    using Iter_t = T*;
    using ConstIter_t = const T*;
    using MemoryPolicy_t = MemoryPolicy;

    //------------------------------------------------------------------------------
    static constexpr Index_t IndexBad()
    {
        return (Index_t)-1;
    }

    //------------------------------------------------------------------------------
    bool IsMovable()
    {
        return MemoryPolicy::IS_MOVABLE;
    }

    //------------------------------------------------------------------------------
    TemplArray() = default;

    //------------------------------------------------------------------------------
    TemplArray(std::initializer_list<T> elements)
    {
        for (auto&& e : elements)
        {
            Add(std::forward<decltype(e)>(e));
        }
    }

    //------------------------------------------------------------------------------
    ~TemplArray()
    {
        for (Index_t i = 0; i < Count(); ++i)
            Data()[i].~T();

        memory_.Free();
    }

    //------------------------------------------------------------------------------
    TemplArray(const TemplArray<T, MemoryPolicy>& other)
    {
        memory_.CopyConstruct(other.memory_);
        CopyRange(Data(), other.Data(), Count());
    }

    //------------------------------------------------------------------------------
    TemplArray<T, MemoryPolicy>& operator=(const TemplArray<T, MemoryPolicy>& other)
    {
        if (this == &other)
            return *this;

        for (Index_t i = 0; i < Count(); ++i)
            Data()[i].~T();

        memory_.Assign(other.memory_);

        CopyRange(Data(), other.Data(), Count());

        return *this;
    }

    //------------------------------------------------------------------------------
    template<class DummyT = void>
    TemplArray(TemplArray<T, MemoryPolicy>&& other,
        typename std::enable_if_t<MemoryPolicy::IS_MOVABLE, DummyT>* = 0)
    {
        memory_.MoveConstruct(std::move(other.memory_));
    }

    //------------------------------------------------------------------------------
    template<class DummyT = TemplArray<T, MemoryPolicy>&>
    typename std::enable_if_t<MemoryPolicy::IS_MOVABLE, DummyT>
    operator=(TemplArray<T, MemoryPolicy>&& other)
    {
        if (this == &other)
            return *this;

        for (Index_t i = 0; i < Count(); ++i)
            Data()[i].~T();

        memory_.MoveAssign(std::move(other.memory_));

        return *this;
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] Index_t Count() const
    {
        return memory_.Count();
    }

    //------------------------------------------------------------------------------
    Index_t Capacity() const
    {
        return memory_.Capacity();
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] bool IsEmpty() const
    {
        return memory_.Count() == 0;
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] const T& operator[](Index_t index) const
    {
        HS_ASSERT(index < Count());
        return memory_.Items()[index];
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] T& operator[](Index_t index)
    {
        HS_ASSERT(index < Count());
        return memory_.Items()[index];
    }

    //------------------------------------------------------------------------------
    template<class ...ArgsT>
    void EmplaceBack(ArgsT&& ...args)
    {
        if (Count() == Capacity())
            memory_.EnsureEmplaceBack();

        HS_ASSERT(Count() < Capacity());
        new(Data() + Count()) T(std::forward<ArgsT>(args)...);
        memory_.CountMut()++;
    }

    //------------------------------------------------------------------------------
    template<class ...ArgsT>
    void Emplace(Index_t index, ArgsT&& ...args)
    {
        HS_ASSERT(index <= Count());
        if (index == Count())
        {
            EmplaceBack(std::forward<ArgsT>(args)...);
            return;
        }

        if (Count() == Capacity())
        {
            memory_.EnsureEmplace(index);
            new(Data() + index) T(std::forward<ArgsT>(args)...);
        }
        else
        {
            // Move items by one to the right
            if constexpr (std::is_trivial_v<T>)
            {
                memmove(&Data()[index + 1], &Data()[index], (Count() - index) * sizeof(T));
            }
            else
            {
                // count_ is at least 1, otherwise there is early exit
                // New Back place is not initialized item, move construct there
                new(Data() + Count()) T(std::move(Data()[Count() - 1]));
                // Other items can be move assigned
                for (T* item = Data() + Count() - 1; item != Data() + index; --item)
                    *item = std::move(*(item - 1));
            }

            Data()[index] = T(std::forward<ArgsT>(args)...);
        }

        HS_ASSERT(Count() < Capacity());
        memory_.CountMut()++;
    }

    //------------------------------------------------------------------------------
    void Add(const T& item)
    {
        // Check for aliasing
        HS_ASSERT((&item < Data() || &item >= Data() + Capacity()) && "Inserting item from array to itself is not handled");
        EmplaceBack(item);
    }

    //------------------------------------------------------------------------------
    void Add(T&& item)
    {
        // Check for aliasing
        HS_ASSERT((&item < Data() || &item >= Data() + Capacity()) && "Inserting item from array to itself is not handled");
        EmplaceBack(std::move(item));
    }

    /*!
    If the item is present in the array returs false, if not present adds it and returns true.
    O(n) complexity where n is the number of elements in the array.
    */
    bool AddUnique(const T& item)
    {
        if (IndexOf(item) != IndexBad())
            return false;

        Add(item);
        return true;
    }

    //------------------------------------------------------------------------------
    void Insert(Index_t index, const T& item)
    {
        // Check for aliasing
        HS_ASSERT((&item < Data() || &item >= Data() + Capacity()) && "Inserting item from array to itself is not handled");
        Emplace(index, item);
    }

    //------------------------------------------------------------------------------
    void Insert(Index_t index, T&& item)
    {
        // Check for aliasing
        HS_ASSERT((&item < Data() || &item >= Data() + Capacity()) && "Inserting item from array to itself is not handled");
        Emplace(index, std::move(item));
    }

    //------------------------------------------------------------------------------
    void Remove(Index_t index)
    {
        HS_ASSERT(index < Count());

        if constexpr (std::is_trivial_v<T>)
        {
            memmove(&Data()[index], &Data()[index + 1], (Count() - index) * sizeof(T));
        }
        else
        {
            for (T* item = Data() + index; item != Data() + Count() - 1; ++item)
                *item = std::move(item[1]);
            Data()[Count() - 1].~T();
        }

        memory_.CountMut()--;
    }

    //------------------------------------------------------------------------------
    void RemoveBack()
    {
        Remove(Count() - 1);
    }

    //------------------------------------------------------------------------------
    void Clear()
    {
        for (Index_t i = 0; i < Count(); ++i)
        {
            Data()[i].~T();
        }
        memory_.CountMut() = 0;
    }

    //------------------------------------------------------------------------------
    void Reserve(Index_t capacity)
    {
        if (capacity <= Capacity())
            return;

        memory_.Grow(capacity);
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] const T& Front() const
    {
        HS_ASSERT(Count());
        return Data()[0];
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] T& Front()
    {
        HS_ASSERT(Count());
        return Data()[0];
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] const T& Back() const
    {
        HS_ASSERT(Count());
        return Data()[Count() - 1];
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] T& Back()
    {
        HS_ASSERT(Count());
        return Data()[Count() - 1];
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] T* Data()
    {
        return memory_.Items();
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] const T* Data() const
    {
        return memory_.Items();
    }

    //------------------------------------------------------------------------------
    Index_t IndexOf(const T& item) const
    {
        for (Index_t i = 0; i < Count(); ++i)
        {
            if (Data()[i] == item)
                return i;
        }

        return IndexBad();
    }

    #pragma region Iterators
    //------------------------------------------------------------------------------
    // Iterators
    //------------------------------------------------------------------------------
    [[nodiscard]] ConstIter_t cbegin() const
    {
        return Data();
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] ConstIter_t begin() const
    {
        return cbegin();
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] Iter_t begin()
    {
        return Data();
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] ConstIter_t cend() const
    {
        return Data() + Count();
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] ConstIter_t end() const
    {
        return cend();
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] Iter_t end()
    {
        return Data() + Count();
    }
    #pragma endregion

private:
    MemoryPolicy memory_;

    void CopyRange(T* dst, const T* src, Index_t count)
    {
        for (Index_t i = 0; i < count; ++i)
        {
            dst[i] = src[i];
        }
    }
};

//------------------------------------------------------------------------------
// Small array
//------------------------------------------------------------------------------
template<class T>
class SmallArrayBase
{
public:
    using Item_t = T;
    using Iter_t = T*;
    using ConstIter_t = const T*;

    //------------------------------------------------------------------------------
    ~SmallArrayBase()
    {
        for (Index_t i = 0; i < Count(); ++i)
            items_[i].~T();

        if (!IsSmall())
        {
            FreeAligned(items_);
        }
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] Index_t Count() const
    {
        return count_;
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] Index_t Capacity() const
    {
        return capacity_;
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] bool IsEmpty() const
    {
        return count_ == 0;
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] T* Data()
    {
        return items_;
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] const T* Data() const
    {
        return items_;
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] const T& operator[](Index_t index) const
    {
        HS_ASSERT(index < Count());
        return items_[index];
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] T& operator[](Index_t index)
    {
        HS_ASSERT(index < Count());
        return items_[index];
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] const T& Front() const
    {
        HS_ASSERT(Count());
        return Data()[0];
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] T& Front()
    {
        HS_ASSERT(Count());
        return Data()[0];
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] const T& Back() const
    {
        HS_ASSERT(Count());
        return Data()[Count() - 1];
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] T& Back()
    {
        HS_ASSERT(Count());
        return Data()[Count() - 1];
    }

    //------------------------------------------------------------------------------
    template<class ...ArgsT>
    void EmplaceBack(ArgsT&& ...args)
    {
        if (Count() >= Capacity())
        {
            Grow();
        }

        HS_ASSERT(Count() < Capacity());
        new(Data() + Count()) T(std::forward<ArgsT>(args)...);
        ++count_;
    }

    //------------------------------------------------------------------------------
    template<class ...ArgsT>
    void Emplace(Index_t index, ArgsT&& ...args)
    {
        HS_ASSERT(index <= Count());
        if (index == Count())
        {
            EmplaceBack(std::forward<ArgsT>(args)...);
            return;
        }

        if (Count() == Capacity())
        {
            GrowForInsert(index);
            new(Data() + index) T(std::forward<ArgsT>(args)...);
        }
        else
        {
            // Move items by one to the right
            if constexpr (std::is_trivial_v<T>)
            {
                memmove(&Data()[index + 1], &Data()[index], (Count() - index) * sizeof(T));
            }
            else
            {
                // count_ is at least 1, otherwise there is early exit
                // New last place is not initialized item, move construct there
                new(Data() + Count()) T(std::move(Data()[Count() - 1]));
                // Other items can be move assigned
                for (T* item = Data() + Count() - 1; item != Data() + index; --item)
                    *item = std::move(*(item - 1));
            }

            Data()[index] = T(std::forward<ArgsT>(args)...);
        }

        HS_ASSERT(Count() < Capacity());
        ++count_;
    }

    //------------------------------------------------------------------------------
    void Add(const T& item)
    {
        // Check for aliasing
        HS_ASSERT((&item < Data() || &item >= Data() + Capacity()) && "Inserting item from array to itself is not handled");
        EmplaceBack(item);
    }

    //------------------------------------------------------------------------------
    void Add(T&& item)
    {
        // Check for aliasing
        HS_ASSERT((&item < Data() || &item >= Data() + Capacity()) && "Inserting item from array to itself is not handled");
        EmplaceBack(std::move(item));
    }

    //------------------------------------------------------------------------------
    void AddRange(Span<T> items)
    {
        Reserve(Count() + items.Count());
        for (int i = 0; i < items.Count(); ++i)
            Add(items[i]);
    }

    //------------------------------------------------------------------------------
    static constexpr Index_t IndexBad()
    {
        return (Index_t)-1;
    }

    //------------------------------------------------------------------------------
    Index_t IndexOf(const T& item) const
    {
        for (Index_t i = 0; i < Count(); ++i)
        {
            if (Data()[i] == item)
                return i;
        }

        return IndexBad();
    }

    /*!
    If the item is present in the array returs false, if not present adds it and returns true.
    O(n) complexity where n is the number of elements in the array.
    */
    bool AddUnique(const T& item)
    {
        if (IndexOf(item) != IndexBad())
            return false;

        Add(item);
        return true;
    }

    //------------------------------------------------------------------------------
    void Insert(Index_t index, const T& item)
    {
        // Check for aliasing
        HS_ASSERT((&item < Data() || &item >= Data() + Capacity()) && "Inserting item from array to itself is not handled");
        Emplace(index, item);
    }

    //------------------------------------------------------------------------------
    void Insert(Index_t index, T&& item)
    {
        // Check for aliasing
        HS_ASSERT((&item < Data() || &item >= Data() + Capacity()) && "Inserting item from array to itself is not handled");
        Emplace(index, std::move(item));
    }

    //------------------------------------------------------------------------------
    void Remove(Index_t index)
    {
        HS_ASSERT(index < Count());

        if constexpr (std::is_trivial_v<T>)
        {
            memmove(&Data()[index], &Data()[index + 1], (Count() - index) * sizeof(T));
        }
        else
        {
            for (T* item = Data() + index; item != Data() + Count() - 1; ++item)
                *item = std::move(item[1]);
            Data()[Count() - 1].~T();
        }

        --count_;
    }

    //------------------------------------------------------------------------------
    void RemoveBack()
    {
        Remove(Count() - 1);
    }

    //------------------------------------------------------------------------------
    void Clear()
    {
        for (Index_t i = 0; i < Count(); ++i)
        {
            Data()[i].~T();
        }
        count_ = 0;
    }

    //------------------------------------------------------------------------------
    void Reserve(Index_t capacity)
    {
        if (capacity <= Capacity())
            return;

        Grow(capacity);
    }

    #pragma region Iterators
    //------------------------------------------------------------------------------
    // Iterators
    //------------------------------------------------------------------------------
    [[nodiscard]] ConstIter_t cbegin() const
    {
        return Data();
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] ConstIter_t begin() const
    {
        return cbegin();
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] Iter_t begin()
    {
        return Data();
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] ConstIter_t cend() const
    {
        return Data() + Count();
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] ConstIter_t end() const
    {
        return cend();
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] Iter_t end()
    {
        return Data() + Count();
    }
    #pragma endregion

protected:
    T* items_;
    Index_t count_{};
    Index_t capacity_;

    //------------------------------------------------------------------------------
    SmallArrayBase(Index_t initialCapacity)
        : items_(reinterpret_cast<T*>(GetSmallData())), capacity_(initialCapacity)
    {
    }

    // Protected so we can test it

    //! Return pointer to the small items array
    void* GetSmallData() const;

    //! Determine whether the array is small or dynamically allocated
    bool IsSmall() const
    {
        return items_ == GetSmallData();
    }

private:
    //------------------------------------------------------------------------------
    Index_t GetNextCapacity()
    {
        auto nextCapacity = static_cast<Index_t>(NextPow2(static_cast<IndexUnsigned_t>(capacity_)));
        HS_ASSERT(nextCapacity > 0);
        return nextCapacity;
    }

    //------------------------------------------------------------------------------
    void Grow()
    {
        Grow(GetNextCapacity());
    }

    //------------------------------------------------------------------------------
    void Grow(Index_t newCapacity)
    {
        capacity_ = newCapacity;
        auto newItems = reinterpret_cast<T*>(AllocAligned(capacity_ * sizeof(T), alignof(T)));

        if constexpr (std::is_trivial_v<T>)
        {
            memcpy(newItems, items_, count_ * sizeof(T));
        }
        else
        {
            for (Index_t i = 0; i < count_; ++i)
            {
                new(newItems + i) T(std::move(items_[i]));
                items_[i].~T();
            }
        }

        if (!IsSmall())
        {
            FreeAligned(items_);
        }

        items_ = newItems;
    }

    //------------------------------------------------------------------------------
    void GrowForInsert(Index_t index)
    {
        capacity_ = GetNextCapacity();

        auto newItems = static_cast<T*>(AllocAligned(sizeof(T) * capacity_, alignof(T)));
        if constexpr (std::is_trivial_v<T>)
        {
            memcpy(newItems, items_, sizeof(T) * index);
            memcpy(&newItems[index + 1], &items_[index], (count_ - index) * sizeof(T));
        }
        else
        {
            for (Index_t i = 0; i < index; ++i)
            {
                new(newItems + i) T(std::move(items_[i]));
                items_[i].~T();
            }
            for (Index_t i = index; i < count_; ++i)
            {
                new(newItems + i + 1) T(std::move(items_[i]));
                items_[i].~T();
            }
        }
        FreeAligned(items_);
        items_ = newItems;
    }
};

//------------------------------------------------------------------------------
//! Offset of the small item array
template <class T/*, typename = void*/>
struct SmallVectorAlignmentAndSize
{
  SmallArrayBase<T> base_;
  alignas(T) uint8 smallData_;
};

//------------------------------------------------------------------------------
template<class T>
void* SmallArrayBase<T>::GetSmallData() const
{
    uintptr thisPtr = reinterpret_cast<uintptr>(this);
    return reinterpret_cast<void*>(thisPtr + offsetof(SmallVectorAlignmentAndSize<T>, smallData_));
}

//------------------------------------------------------------------------------
template<class T, Index_t CAPACITY>
class SmallArrayStorage
{
protected:
    alignas(T) uint8 smallItems_[CAPACITY * sizeof(T)];
};

//------------------------------------------------------------------------------
// TODO(pavel): Is the alignas necessary here? If we enable it the size of the
// SmallArray increases.
template<class T>
class /*alignas(T)*/ SmallArrayStorage<T, 0> { };

//------------------------------------------------------------------------------
template<class T, Index_t SMALL_CAPACITY>
class SmallArray : public SmallArrayBase<T>, public SmallArrayStorage<T, SMALL_CAPACITY>
{
protected:
    using This_t = SmallArray<T, SMALL_CAPACITY>;
    using SmallArrayBase<T>::items_;
    using SmallArrayBase<T>::count_;
    using SmallArrayBase<T>::capacity_;
    using SmallArrayBase<T>::IsSmall;

public:
    using SmallArrayBase<T>::Reserve;
    using SmallArrayBase<T>::Add;

    //------------------------------------------------------------------------------
    bool IsMovable()
    {
        return !IsSmall();
    }

    //------------------------------------------------------------------------------
    SmallArray() : SmallArrayBase<T>(SMALL_CAPACITY)
    {
        // For SMALL_CAPACITY == 0 we don't need the offset
        if constexpr (SMALL_CAPACITY != 0)
        {
            // Check if our compiler-specific hacks work
            static_assert(offsetof(This_t, smallItems_) == offsetof(SmallVectorAlignmentAndSize<T>, smallData_));
        }
    }

    //------------------------------------------------------------------------------
    SmallArray(std::initializer_list<T> elements)
        : SmallArrayBase<T>(SMALL_CAPACITY)
    {
        Reserve(elements.size());
        for (auto&& e : elements)
        {
            Add(std::forward<decltype(e)>(e));
        }
    }

    //------------------------------------------------------------------------------
    SmallArray(const SmallArray<T, SMALL_CAPACITY>& other)
        : SmallArrayBase<T>(SMALL_CAPACITY)
    {
        capacity_ = other.capacity_;
        count_ = other.count_;

        if (!other.IsSmall())
        {
            items_ = reinterpret_cast<T*>(AllocAligned(other.capacity_ * sizeof(T), alignof(T)));
        }

        for (Index_t i = 0; i < count_; ++i)
        {
            new(items_ + i) T(other.items_[i]);
        }
    }

    //------------------------------------------------------------------------------
    SmallArray(SmallArray<T, SMALL_CAPACITY>&& other)
        : SmallArrayBase<T>(SMALL_CAPACITY)
    {
        capacity_ = other.capacity_;
        count_ = other.count_;

        if (!other.IsSmall())
        {
            items_ = other.items_;
            other.items_ = nullptr;
        }
        else
        {
            // Cannot move internal buffer, fallback to copy
            for (Index_t i = 0; i < count_; ++i)
            {
                new(items_ + i) T(std::move(other.items_[i]));
            }
        }

        other.count_ = 0;
    }

    //------------------------------------------------------------------------------
    SmallArray<T, SMALL_CAPACITY>& operator=(const SmallArray<T, SMALL_CAPACITY>& other)
    {
        if (this == &other)
            return *this;

        for (Index_t i = 0; i < count_; ++i)
        {
            items_[i].~T();
        }

        if (capacity_ != other.capacity_ && !IsSmall())
        {
            FreeAligned(items_);
        }

        capacity_ = other.capacity_;
        count_ = other.count_;

        if (!other.IsSmall())
        {
            items_ = reinterpret_cast<T*>(AllocAligned(capacity_ * sizeof(T), alignof(T)));
        }

        for (Index_t i = 0; i < count_; ++i)
        {
            new(items_ + i) T(other.items_[i]);
        }

        return *this;
    }

    //------------------------------------------------------------------------------
    SmallArray<T, SMALL_CAPACITY>& operator=(SmallArray<T, SMALL_CAPACITY>&& other)
    {
        if (this == &other)
            return *this;

        for (Index_t i = 0; i < count_; ++i)
        {
            items_[i].~T();
        }

        if (capacity_ != other.capacity_ && !IsSmall())
        {
            FreeAligned(items_);
        }

        capacity_ = other.capacity_;
        count_ = other.count_;

        if (!other.IsSmall())
        {
            items_ = other.items_;
        }
        else
        {
            // Cannot move internal buffer, fallback to copy
            for (Index_t i = 0; i < count_; ++i)
            {
                new(items_ + i) T(std::move(other.items_[i]));
            }
        }

        return *this;
    }
};

//------------------------------------------------------------------------------
template<class T>
//using Array = TemplArray<T, GrowableMemoryPolicy<T>>;
using Array = SmallArray<T, 0>;

//------------------------------------------------------------------------------
template<class T, Index_t capacity>
using StaticArray = TemplArray<T, StaticMemoryPolicy<T, capacity>>;

//------------------------------------------------------------------------------
template<class T, class MemoryPolicyT>
Span<T> MakeSpan(TemplArray<T, MemoryPolicyT>& array)
{
    return Span<T>(array.Data(), array.Count());
}

//------------------------------------------------------------------------------
template<class T, class MemoryPolicyT>
Span<const T> MakeSpan(const TemplArray<T, MemoryPolicyT>& array)
{
    return Span<const T>(array.Data(), array.Count());
}

//------------------------------------------------------------------------------
template<class T>
Span<T> MakeSpan(SmallArrayBase<T>& array)
{
    return Span<T>(array.Data(), array.Count());
}

//------------------------------------------------------------------------------
template<class T>
Span<const T> MakeSpan(const SmallArrayBase<T>& array)
{
    return Span<const T>(array.Data(), array.Count());
}

}
