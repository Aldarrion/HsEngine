#pragma once

namespace hs
{

//------------------------------------------------------------------------------
template<class T1, class T2>
struct Pair
{
    T1 first;
    T2 second;

    //------------------------------------------------------------------------------
    Pair() = default;

    //------------------------------------------------------------------------------
    Pair(const T1& first, const T2& second)
        : first(first), second(second)
    {
    }

    //------------------------------------------------------------------------------
    Pair(std::remove_reference_t<T1>&& first, std::remove_reference<T2>&& second)
        : first(std::forward(first)), second(std::forward(second))
    {
    }

    //------------------------------------------------------------------------------
    const T1& Key() const
    {
        return first;
    }

    //------------------------------------------------------------------------------
    T1& Key()
    {
        return first;
    }

    //------------------------------------------------------------------------------
    const T2& Value() const
    {
        return second;
    }

    //------------------------------------------------------------------------------
    T2& Value()
    {
        return second;
    }
};

//------------------------------------------------------------------------------
template<class T1, class T2>
Pair<T1, T2> MakePair(T1&& first, T2&& second)
{
    return Pair<T1, T2>(std::forward(first), std::forward(second));
}

}
