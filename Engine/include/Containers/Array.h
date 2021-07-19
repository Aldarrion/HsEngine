#pragma once

#include "Config.h"

#include "System/Memory.h"

#include "Containers/Span.h"

#include "Math/Math.h"
#include "Common/Types.h"
#include "Common/Assert.h"

#include <cstring>
#include <type_traits>
#include <initializer_list>
#include <new>

namespace hs
{

/*!
Unsigned types come with a lot of hassle
64-bit by default is wasteful but more importantly it puts sign extension to
many places where 32/64-bit is mixed.

If we'll need more than 2G elements in an array we can make this a template
argument and use int64.
*/
using ArrayIndex_t = int;

//------------------------------------------------------------------------------
template<class T, ArrayIndex_t capacity_>
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
        HS_ASSERT(false && "Static array size cannot grow");
    }

    //------------------------------------------------------------------------------
    void EnsureEmplace(ArrayIndex_t index)
    {
        HS_ASSERT(false && "Static array size cannot grow");
    }

    //------------------------------------------------------------------------------
    void Grow(ArrayIndex_t capacity)
    {
        HS_ASSERT(false && "Static array size cannot grow");
    }

    //------------------------------------------------------------------------------
    ArrayIndex_t& CountMut()
    {
        return count_;
    }

    //------------------------------------------------------------------------------
    ArrayIndex_t Count() const
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
    ArrayIndex_t Capacity() const
    {
        return capacity_;
    }

private:
    alignas(T) uint8 items_[capacity_ * sizeof(T)];
    ArrayIndex_t count_{};
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
    void EnsureEmplace(ArrayIndex_t index)
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
            for (ArrayIndex_t i = 0; i < index; ++i)
            {
                new(newItems + i) T(std::move(items_[i]));
                items_[i].~T();
            }
            for (ArrayIndex_t i = index; i < count_; ++i)
            {
                new(newItems + i + 1) T(std::move(items_[i]));
                items_[i].~T();
            }
        }
        FreeAligned(items_);
        items_ = newItems;
    }

    //------------------------------------------------------------------------------
    void Grow(ArrayIndex_t capacity)
    {
        capacity_ = Max(capacity, MIN_CAPACITY);
        auto newItems = static_cast<T*>(AllocAligned(sizeof(T) * capacity_, alignof(T)));
        if constexpr (std::is_trivial_v<T>)
        {
            memcpy(newItems, items_, sizeof(T) * count_);
        }
        else
        {
            for (ArrayIndex_t i = 0; i < count_; ++i)
            {
                new(newItems + i) T(std::move(items_[i]));
                items_[i].~T();
            }
        }

        FreeAligned(items_);
        items_ = newItems;
    }

    //------------------------------------------------------------------------------
    ArrayIndex_t& CountMut()
    {
        return count_;
    }

    //------------------------------------------------------------------------------
    ArrayIndex_t Count() const
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
    ArrayIndex_t Capacity() const
    {
        return capacity_;
    }

private:
    static constexpr ArrayIndex_t MIN_CAPACITY = 8;

    ArrayIndex_t capacity_{};
    ArrayIndex_t count_{};
    T* items_{};

    //------------------------------------------------------------------------------
    ArrayIndex_t GetNextCapacity() const
    {
        ArrayIndex_t newCapacity = Max(NextPow2(static_cast<uint>(capacity_)), static_cast<uint>(MIN_CAPACITY));
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
    using ConstIter_t = const Iter_t;
    using MemoryPolicy_t = MemoryPolicy;

    //------------------------------------------------------------------------------
    static constexpr ArrayIndex_t IndexBad()
    {
        return (ArrayIndex_t)-1;
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
        for (ArrayIndex_t i = 0; i < Count(); ++i)
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

        for (ArrayIndex_t i = 0; i < Count(); ++i)
            Data()[i].~T();

        memory_.Assign(other.memory_);

        CopyRange(Data(), other.Data(), Count());

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

        for (ArrayIndex_t i = 0; i < Count(); ++i)
            Data()[i].~T();

        memory_.MoveAssign(std::move(other.memory_));

        return *this;
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] ArrayIndex_t Count() const
    {
        return memory_.Count();
    }

    //------------------------------------------------------------------------------
    ArrayIndex_t Capacity() const
    {
        return memory_.Capacity();
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] bool IsEmpty() const
    {
        return memory_.Count() == 0;
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] const T& operator[](ArrayIndex_t index) const
    {
        HS_ASSERT(index < Count());
        return memory_.Items()[index];
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] T& operator[](ArrayIndex_t index)
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
    void Emplace(ArrayIndex_t index, ArgsT&& ...args)
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
                // New last place is not initialized item, move construct there
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
        for (ArrayIndex_t i = 0; i < Count(); ++i)
        {
            if (Data()[i] == item)
                return false;
        }

        Add(item);
        return true;
    }

    //------------------------------------------------------------------------------
    void Insert(ArrayIndex_t index, const T& item)
    {
        // Check for aliasing
        HS_ASSERT((&item < Data() || &item >= Data() + Capacity()) && "Inserting item from array to itself is not handled");
        Emplace(index, item);
    }

    //------------------------------------------------------------------------------
    void Insert(ArrayIndex_t index, T&& item)
    {
        // Check for aliasing
        HS_ASSERT((&item < Data() || &item >= Data() + Capacity()) && "Inserting item from array to itself is not handled");
        Emplace(index, std::move(item));
    }

    //------------------------------------------------------------------------------
    void Remove(ArrayIndex_t index)
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
    void RemoveLast()
    {
        Remove(Count() - 1);
    }

    //------------------------------------------------------------------------------
    void Clear()
    {
        for (ArrayIndex_t i = 0; i < Count(); ++i)
        {
            Data()[i].~T();
        }
        memory_.CountMut() = 0;
    }

    //------------------------------------------------------------------------------
    void Reserve(ArrayIndex_t capacity)
    {
        if (capacity <= Capacity())
            return;

        memory_.Grow(capacity);
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] const T& First() const
    {
        HS_ASSERT(Count());
        return Data()[0];
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] T& First()
    {
        HS_ASSERT(Count());
        return Data()[0];
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] const T& Last() const
    {
        HS_ASSERT(Count());
        return Data()[Count() - 1];
    }

    //------------------------------------------------------------------------------
    [[nodiscard]] T& Last()
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
    ArrayIndex_t IndexOf(const T& item) const
    {
        for (ArrayIndex_t i = 0; i < Count(); ++i)
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

    void CopyRange(T* dst, const T* src, ArrayIndex_t count)
    {
        for (ArrayIndex_t i = 0; i < count; ++i)
        {
            dst[i] = src[i];
        }
    }
};

//------------------------------------------------------------------------------
template<class T>
using Array = TemplArray<T, GrowableMemoryPolicy<T>>;

//------------------------------------------------------------------------------
template<class T, ArrayIndex_t capacity>
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
