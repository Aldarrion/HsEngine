#pragma once

#include "Config.h"
#include "Common/Assert.h"

#if HS_MSVC
    #include <intrin.h>
#elif HS_CLANG || HS_GCC
#endif

namespace hs
{

//------------------------------------------------------------------------------
//! Atomically increments value and returns the result
inline int AtomicIncrement(int* value)
{
    #if HS_MSVC
        return _InterlockedIncrement(reinterpret_cast<long*>(value));
    #elif HS_CLANG || HS_GCC
        return __atomic_add_fetch(value, 1, __ATOMIC_SEQ_CST);
    #else
        HS_NOT_IMPLEMENTED
    #endif
}

//------------------------------------------------------------------------------
//! Atomically decrements value and returns the result
inline int AtomicDecrement(int* value)
{
    #if HS_MSVC
        return _InterlockedDecrement(reinterpret_cast<long*>(value));
    #elif HS_CLANG || HS_GCC
        return __atomic_sub_fetch(value, 1, __ATOMIC_SEQ_CST);
    #else
        HS_NOT_IMPLEMENTED
    #endif
}

//------------------------------------------------------------------------------
//! Atomically adds x to value and returns the result
inline int AtomicAdd(int* value, int x)
{
    #if HS_MSVC
        return _InterlockedExchangeAdd(reinterpret_cast<long*>(value), x);
    #elif HS_CLANG || HS_GCC
        return __atomic_add_fetch(value, x, __ATOMIC_SEQ_CST);
    #else
        HS_NOT_IMPLEMENTED
    #endif
}

//------------------------------------------------------------------------------
//! Atomically subtracts x from value and returns the result
inline int AtomicSubtract(int* value, int x)
{
    #if HS_MSVC
        return _InterlockedExchangeAdd(reinterpret_cast<long*>(value), -x);
    #elif HS_CLANG || HS_GCC
        return __atomic_sub_fetch(value, x, __ATOMIC_SEQ_CST);
    #else
        HS_NOT_IMPLEMENTED
    #endif
}

//------------------------------------------------------------------------------
//! Atomically loads value and returns it
inline int AtomicLoad(const int* value)
{
    #if HS_MSVC
        int tmp = *value;
        // Barrier after read to simulate acquire semantics
        // = no memory operations after AtomicLoad can be reodered up (before it)
        _ReadWriteBarrier();
        return tmp;
    #elif HS_CLANG || HS_GCC
        return __atomic_load_n(value, __ATOMIC_SEQ_CST);
    #else
        HS_NOT_IMPLEMENTED
    #endif
}

//------------------------------------------------------------------------------
//! Atomically stores x to value
inline void AtomicStore(int* value, int x)
{
    #if HS_MSVC
        _InterlockedExchange(reinterpret_cast<long*>(value), x);
    #elif HS_CLANG || HS_GCC
        __atomic_store_n(value, x, __ATOMIC_SEQ_CST);
    #else
        HS_NOT_IMPLEMENTED
    #endif
}

//------------------------------------------------------------------------------
//! Atomically stores x to value returning the old value
inline int AtomicExchange(int* value, int x)
{
    #if HS_MSVC
        return _InterlockedExchange(reinterpret_cast<long*>(value), x);
    #elif HS_CLANG || HS_GCC
        return __atomic_exchange_n(value, x, __ATOMIC_SEQ_CST);
    #else
        HS_NOT_IMPLEMENTED
    #endif
}

//------------------------------------------------------------------------------
/*!
Atomically compares expected to *dst
if equal, desired is stored to *dst
otherwise, no operation is performed

\return Initial value of *dst before exchange
*/
inline int AtomicCompareExchange(int* dst, int desired, int expected)
{
    #if HS_MSVC
        return _InterlockedCompareExchange(reinterpret_cast<long*>(dst), desired, expected);
    #elif HS_CLANG || HS_GCC
        __atomic_compare_exchange_n(dst, &expected, desired, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
        return expected;
    #else
        HS_NOT_IMPLEMENTED
    #endif
}

//------------------------------------------------------------------------------
class AtomicInt
{
public:
    int Increment();
    int Decrement();
    int Add(int x);
    int Subtract(int x);
    int GetValue() const;
    void SetValue(int x);
    int CompareExchange(int desired, int expected);

private:
    int value_{};
};

//------------------------------------------------------------------------------
inline int AtomicInt::Increment()
{
    return AtomicIncrement(&value_);
}
//------------------------------------------------------------------------------
inline int AtomicInt::Decrement()
{
    return AtomicDecrement(&value_);
}
//------------------------------------------------------------------------------
inline int AtomicInt::Add(int x)
{
    return AtomicAdd(&value_, x);
}
//------------------------------------------------------------------------------
inline int AtomicInt::Subtract(int x)
{
    return AtomicSubtract(&value_, x);
}
//------------------------------------------------------------------------------
inline int AtomicInt::GetValue() const
{
    return AtomicLoad(&value_);
}
//------------------------------------------------------------------------------
inline void AtomicInt::SetValue(int x)
{
    AtomicStore(&value_, x);
}
//------------------------------------------------------------------------------
inline int AtomicInt::CompareExchange(int desired, int expected)
{
    return AtomicCompareExchange(&value_, desired, expected);
}

}
