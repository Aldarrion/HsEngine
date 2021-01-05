#pragma once

#include "Common/Types.h"

#include <cstdlib>
#include <cstring>

namespace hs
{

//------------------------------------------------------------------------------
class String
{
public:
    using Iter_t = char*;
    using ConstIter_t = const Iter_t;

    static String FromFormat(uint maxLen, const char* formatString, ...);

    String() = default;
    explicit String(uint reservedCapacity);
    template<uint N> explicit constexpr String(const char (&stringLiteral)[N]);

    ~String();

    String(const String& other);
    String& operator=(const String& other);
    String(String&& other);
    String& operator=(String&& other);

    [[nodiscard]] const char* Data() const;
    [[nodiscard]] char* Data();

    [[nodiscard]] uint Size() const;

    [[nodiscard]] ConstIter_t cbegin() const;
    [[nodiscard]] ConstIter_t begin() const;
    [[nodiscard]] Iter_t begin();
    [[nodiscard]] ConstIter_t cend() const;
    [[nodiscard]] ConstIter_t end() const;
    [[nodiscard]] Iter_t end();

private:
    char* string_{};
    uint capacity_{};
    uint size_{};
};

//------------------------------------------------------------------------------
template<uint N>
constexpr String::String(const char (&stringLiteral)[N])
{
    capacity_ = N - 1;
    string_ = (char*)malloc(capacity_);
    size_ = capacity_;

    memcpy(string_, stringLiteral, capacity_);
}

//------------------------------------------------------------------------------
uint String::Size() const
{
    return size_;
}

//------------------------------------------------------------------------------
const char* String::Data() const
{
    return string_;
}

//------------------------------------------------------------------------------
char* String::Data()
{
    return string_;
}

//------------------------------------------------------------------------------
String::ConstIter_t String::cbegin() const
{
    return string_;
}

//------------------------------------------------------------------------------
String::ConstIter_t String::begin() const
{
    return cbegin();
}

//------------------------------------------------------------------------------
String::Iter_t String::begin()
{
    return string_;
}

//------------------------------------------------------------------------------
String::ConstIter_t String::cend() const
{
    return string_ + size_;
}

//------------------------------------------------------------------------------
String::ConstIter_t String::end() const
{
    return cend();
}

//------------------------------------------------------------------------------
String::Iter_t String::end()
{
    return string_ + size_;
}

}
