#pragma once

#include "Config.h"
#include <malloc.h>

#if HS_WINDOWS
    #define HS_ALLOCA(Type, count) (Type*)_alloca(count * sizeof(Type))
#elif HS_LINUX
    #include <alloca.h>
    #define HS_ALLOCA(Type, count) (Type*)alloca(count * sizeof(Type))
#endif

namespace hs
{

//------------------------------------------------------------------------------
template<class T>
using RemoveCvRef_t = std::remove_cv_t<std::remove_reference_t<T>>;

//------------------------------------------------------------------------------
template<class T>
void Swap(T& a, T& b)
{
    auto tmp = std::move(a);
    a = std::move(b);
    b = std::move(tmp);
}

}
