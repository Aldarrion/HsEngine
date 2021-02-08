#pragma once

#include "Config.h"
#include "Common/hs_Assert.h"

#if HS_MSVC
    #include <intrin.h>
#elif HS_CLANG || HS_GCC
#endif

namespace hs
{

//------------------------------------------------------------------------------
//! Atomically increments value and returns the result
inline int AtomicIncrement(volatile int* value)
{
    #if HS_MSVC
        return _InterlockedIncrement(reinterpret_cast<volatile long*>(value));
    #elif HS_CLANG || HS_GCC
        return __atomic_add_fetch(&value, 1, __ATOMIC_SEQ_CST);
    #else
        HS_NOT_IMPLEMENTED
    #endif
}

//------------------------------------------------------------------------------
//! Atomically decrements value and returns the result
inline int AtomicDecrement(volatile int* value)
{
    #if HS_MSVC
        return _InterlockedDecrement(reinterpret_cast<volatile long*>(value));
    #elif HS_CLANG || HS_GCC
        return __atomic_sub_fetch(value, 1, __ATOMIC_SEQ_CST);
    #else
        HS_NOT_IMPLEMENTED
    #endif
}

//------------------------------------------------------------------------------
//! Atomically adds x to value and returns the result
inline int AtomicAdd(volatile int* value, int x)
{
    #if HS_MSVC
        return _InterlockedExchangeAdd(reinterpret_cast<volatile long*>(value), x);
    #elif HS_CLANG || HS_GCC
        return __atomic_add_fetch(value, x, __ATOMIC_SEQ_CST);
    #else
        HS_NOT_IMPLEMENTED
    #endif
}

//------------------------------------------------------------------------------
//! Atomically subtracts x from value and returns the result
inline int AtomicSub(volatile int* value, int x)
{
    #if HS_MSVC
        return _InterlockedExchangeAdd(reinterpret_cast<volatile long*>(value), -x);
    #elif HS_CLANG || HS_GCC
        return __atomic_sub_fetch(value, x, __ATOMIC_SEQ_CST);
    #else
        HS_NOT_IMPLEMENTED
    #endif
}

//------------------------------------------------------------------------------
//! Atomically loads value and returns it
inline int AtomicLoad(const volatile int* value)
{
    #if HS_MSVC
        return *value;
    #elif HS_CLANG || HS_GCC
        return __atomic_load_n(value, __ATOMIC_SEQ_CST);
    #else
        HS_NOT_IMPLEMENTED
    #endif
}

//------------------------------------------------------------------------------
//! Atomically stores x to value
inline void AtomicStore(volatile int* value, int x)
{
    #if HS_MSVC
         *value = x;
    #elif HS_CLANG || HS_GCC
        __atomic_store_n(value, x, __ATOMIC_SEQ_CST);
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
inline int AtomicCompareExchange(volatile int* dst, int desired, int expected)
{
    #if HS_MSVC
        return _InterlockedCompareExchange(reinterpret_cast<volatile long*>(dst), desired, expected);
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
    int Sub(int x);
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
inline int AtomicInt::Sub(int x)
{
    return AtomicSub(&value_, x);
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
