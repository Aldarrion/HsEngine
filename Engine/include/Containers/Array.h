#pragma once

#include "Config.h"

#include "Containers/Span.h"

#include "Common/Types.h"
#include "Common/hs_Assert.h"

#include <cstdlib>
#include <cstring>
#include <type_traits>
#include <initializer_list>
#include <new>

namespace hs
{

//------------------------------------------------------------------------------
template<class T, uint64 capacity_>
class StaticMemoryPolicy
{
public:
    static constexpr bool IS_MOVEABLE = false;

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
        hs_assert(false && "Static array size cannot grow");
    }

    //------------------------------------------------------------------------------
    void EnsureEmplace(uint64 index)
    {
        hs_assert(false && "Static array size cannot grow");
    }

    //------------------------------------------------------------------------------
    void Reserve(uint64 capacity)
    {
        hs_assert(false && "Static array size cannot grow");
    }

    //------------------------------------------------------------------------------
    uint64& CountMut()
    {
        return count_;
    }

    //------------------------------------------------------------------------------
    uint64 Count() const
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
    uint64 Capacity() const
    {
        return capacity_;
    }

private:
    T items_[capacity_];
    uint64 count_;
};

//------------------------------------------------------------------------------
template<class T>
class GrowableMemoryPolicy
{
public:
    static constexpr bool IS_MOVEABLE = true;

    //------------------------------------------------------------------------------
    void CopyConstruct(const GrowableMemoryPolicy<T>& other)
    {
        hs_assert(!items_);

        capacity_ = other.capacity_;
        count_ = other.count_;
        items_ = (T*)malloc(sizeof(T) * capacity_);
    }

    //------------------------------------------------------------------------------
    void Assign(const GrowableMemoryPolicy<T>& other)
    {
        if (capacity_ != other.capacity_)
            items_ = (T*)realloc(items_, sizeof(T) * other.capacity_);

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
        free(items_);
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
        free(items_);
    }

    //------------------------------------------------------------------------------
    void EnsureEmplaceBack()
    {
        capacity_ = ArrMax(capacity_ << 1, MIN_CAPACITY);
        Reserve(capacity_);
    }

    //------------------------------------------------------------------------------
    void EnsureEmplace(uint64 index)
    {
        capacity_ = ArrMax(capacity_ << 1, MIN_CAPACITY);

        auto newItems = (T*)malloc(sizeof(T) * capacity_);
        if constexpr (std::is_trivial_v<T>)
        {
            memcpy(newItems, items_, sizeof(T) * index);
            memcpy(&newItems[index + 1], &items_[index], (count_ - index) * sizeof(T));
        }
        else
        {
            for (int i = 0; i < index; ++i)
            {
                new(newItems + i) T(std::move(items_[i]));
                items_[i].~T();
            }
            for (int i = index; i < count_; ++i)
            {
                new(newItems + i + 1) T(std::move(items_[i]));
                items_[i].~T();
            }
        }
        free(items_);
        items_ = newItems;
    }

    //------------------------------------------------------------------------------
    void Reserve(uint64 capacity)
    {
        capacity_ = ArrMax(capacity, MIN_CAPACITY);
        T* newItems = (T*)malloc(sizeof(T) * capacity_);
        if constexpr (std::is_trivial_v<T>)
        {
            memcpy(newItems, items_, sizeof(T) * count_);
        }
        else
        {
            for (int i = 0; i < count_; ++i)
            {
                new(newItems + i) T(std::move(items_[i]));
                items_[i].~T();
            }
        }

        free(items_);
        items_ = newItems;
    }

    //------------------------------------------------------------------------------
    uint64& CountMut()
    {
        return count_;
    }

    //------------------------------------------------------------------------------
    uint64 Count() const
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
    uint64 Capacity() const
    {
        return capacity_;
    }

private:
    static constexpr uint64 MIN_CAPACITY = 8;

    uint64 capacity_{};
    uint64 count_{};
    T* items_{};

    //------------------------------------------------------------------------------
    uint64 ArrMax(uint64 a, uint64 b)
    {
        return a > b ? a : b;
    }
};

//------------------------------------------------------------------------------
template<class T, class MemoryPolicy>
class TemplArray
{
public:
    using Iter_t = T*;
    using ConstIter_t = const Iter_t;

    //------------------------------------------------------------------------------
    static constexpr uint64 IndexBad()
    {
        return (uint64)-1;
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
        for (int i = 0; i < Count(); ++i)
            Data()[i].~T();

        memory_.Free();
    }

    //------------------------------------------------------------------------------
    TemplArray(const TemplArray<T, MemoryPolicy>& other)
    {
        memory_.CopyConstruct(other.memory_);
        for (int i = 0; i < Count(); ++i)
        {
            Data()[i] = other.Data()[i];
        }
    }

    //------------------------------------------------------------------------------
    TemplArray<T, MemoryPolicy>& operator=(const TemplArray<T, MemoryPolicy>& other)
    {
        if (this == &other)
            return *this;

        for (int i = 0; i < Count(); ++i)
            Data()[i].~T();

        memory_.Assign(other.memory_);

        for (int i = 0; i < Count(); ++i)
        {
            Data()[i] = other.Data()[i];
        }

        return *this;
    }

    //------------------------------------------------------------------------------
    template<class = std::enable_if_t<MemoryPolicy::IS_MOVEABLE, void>>
    TemplArray(TemplArray<T, MemoryPolicy>&& other)
    {
        memory_.MoveConstruct(std::move(other.memory_));
    }

    //------------------------------------------------------------------------------
    template<class = std::enable_if_t<MemoryPolicy::IS_MOVEABLE, void>>
    TemplArray<T, MemoryPolicy>& operator=(TemplArray<T, MemoryPolicy>&& other)
    {
        if (this == &other)
            return *this;

        for (int i = 0; i < Count(); ++i)
            Data()[i].~T();

        memory_.MoveAssign(std::move(other.memory_));

        return *this;
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] uint64 Count() const
    {
        return memory_.Count();
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] bool IsEmpty() const
    {
        return memory_.Count() == 0;
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] const T& operator[](uint64 index) const
    {
        hs_assert(index < Count());
        return memory_.Items()[index];
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] T& operator[](uint64 index)
    {
        hs_assert(index < Count());
        return memory_.Items()[index];
    }

    //------------------------------------------------------------------------------
    template<class ...ArgsT>
    void EmplaceBack(ArgsT&& ...args)
    {
        if (Count() == Capacity())
            memory_.EnsureEmplaceBack();

        hs_assert(Count() < Capacity());
        new(Data() + Count()) T(std::forward<ArgsT>(args)...);
        memory_.CountMut()++;
    }

    //------------------------------------------------------------------------------
    template<class ...ArgsT>
    void Emplace(uint64 index, ArgsT&& ...args)
    {
        hs_assert(index <= Count());
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
                // New last place is not initialized item, move construct there
                new(Data() + Count()) T(std::move(Data()[Count() - 1]));
                // Other items can be move assigned
                for (T* item = Data() + Count() - 1; item != Data() + index; --item)
                    *item = std::move(*(item - 1));
            }

            Data()[index] = T(std::forward<ArgsT>(args)...);
        }

        hs_assert(Count() < Capacity());
        memory_.CountMut()++;
    }

    //------------------------------------------------------------------------------
    void Add(const T& item)
    {
        // Check for aliasing
        hs_assert((&item < Data() || &item >= Data() + Capacity()) && "Inserting item from array to itself is not handled");
        EmplaceBack(item);
    }

    //------------------------------------------------------------------------------
    void Add(T&& item)
    {
        // Check for aliasing
        hs_assert((&item < Data() || &item >= Data() + Capacity()) && "Inserting item from array to itself is not handled");
        EmplaceBack(std::move(item));
    }

    /*!
    If the item is present in the array returs false, if not present adds it and returns true.
    O(n) complexity where n is the number of elements in the array.
    */
    bool AddUnique(const T& item)
    {
        for (int i = 0; i < Count(); ++i)
        {
            if (Data()[i] == item)
                return false;
        }

        Add(item);
        return true;
    }

    //------------------------------------------------------------------------------
    void Insert(uint64 index, const T& item)
    {
        // Check for aliasing
        hs_assert((&item < Data() || &item >= Data() + Capacity()) && "Inserting item from array to itself is not handled");
        Emplace(index, item);
    }

    //------------------------------------------------------------------------------
    void Insert(uint64 index, T&& item)
    {
        // Check for aliasing
        hs_assert((&item < Data() || &item >= Data() + Capacity()) && "Inserting item from array to itself is not handled");
        Emplace(index, std::move(item));
    }

    //------------------------------------------------------------------------------
    void Remove(uint64 index)
    {
        hs_assert(index < Count());

        if (std::is_trivial_v<T>)
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
    void RemoveLast()
    {
        Remove(Count() - 1);
    }

    //------------------------------------------------------------------------------
    void Clear()
    {
        for (uint64 i = 0; i < Count(); ++i)
        {
            Data()[i].~T();
        }
        memory_.CountMut() = 0;
    }

    //------------------------------------------------------------------------------
    void Reserve(uint64 capacity)
    {
        if (capacity <= Capacity())
            return;

        memory_.Reserve(capacity);
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] const T& First() const
    {
        hs_assert(Count());
        return Data()[0];
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] T& First()
    {
        hs_assert(Count());
        return Data()[0];
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] const T& Last() const
    {
        hs_assert(Count());
        return Data()[Count() - 1];
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] T& Last()
    {
        hs_assert(Count());
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
    uint64 IndexOf(const T& item) const
    {
        for (int i = 0; i < Count(); ++i)
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

    uint64 Capacity() const
    {
        return memory_.Capacity();
    }
};

//------------------------------------------------------------------------------
template<class T>
using Array = TemplArray<T, GrowableMemoryPolicy<T>>;

//------------------------------------------------------------------------------
template<class T, uint64 capacity>
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

}
