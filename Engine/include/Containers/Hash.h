#pragma once

#include "Config.h"
#include "Common/Types.h"

#include <cstring>

namespace hs
{

using Hash_t = uint64;

//------------------------------------------------------------------------------
template<class KeyT>
struct FibonacciHash
{
    constexpr Hash_t operator()(const KeyT key) const
    {
        return (key * 11400714819323198485llu) >> 61;
    }
};

//------------------------------------------------------------------------------
template<typename>
struct StrCmpEq
{
    constexpr bool operator()(const char* lhs, const char* rhs) const
    {
        return strcmp(lhs, rhs) == 0;
    }
};

//------------------------------------------------------------------------------
template<typename>
struct StrHash
{
    Hash_t operator()(const char* key) const
    {
        Hash_t hash = 9909453657034508789u;
        uint len = strlen(key);
        for (uint i = 0; i < len; ++i)
        {
            hash = hash * 1525334644490293591u + key[i];
        }
        return hash;
    }
};

//------------------------------------------------------------------------------
template<class>
struct DefaultHash
{
    //static_assert(false, "The DefaultHash is not specialized for this type");
};

//------------------------------------------------------------------------------
template<>
struct DefaultHash<int>
{
    static Hash_t Hash(int x)
    {
        return FibonacciHash<int>()(x);
    }
};

//------------------------------------------------------------------------------
template<>
struct DefaultHash<int64>
{
    static Hash_t Hash(int64 x)
    {
        return FibonacciHash<int64>()(x);
    }
};

//------------------------------------------------------------------------------
template<>
struct DefaultHash<uint>
{
    static Hash_t Hash(uint x)
    {
        return FibonacciHash<uint>()(x);
    }
};

//------------------------------------------------------------------------------
template<>
struct DefaultHash<uint64>
{
    static Hash_t Hash(uint64 x)
    {
        return FibonacciHash<uint64>()(x);
    }
};

}
