#pragma once

#include <type_traits>

namespace hs
{

//------------------------------------------------------------------------------
template<class T, class EnableT = void>
struct ArgumentTypeTempl;

//------------------------------------------------------------------------------
template<class T>
struct ArgumentTypeTempl<T, typename std::enable_if_t<(!std::is_trivial_v<T> || sizeof(T) > 8), void>>
{
    using Type = const T&;
};

//------------------------------------------------------------------------------
template<class T>
struct ArgumentTypeTempl<T, typename std::enable_if_t<(std::is_trivial_v<T> && sizeof(T) <= 8), void>>
{
    using Type = T;
};

}
