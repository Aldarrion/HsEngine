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

    [[nodiscard]] bool IsEmpty() const;
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
class StringView
{
public:
    using Iter_t = const char*;
    using ConstIter_t = Iter_t;

    StringView() = default;
    template<uint N> explicit constexpr StringView(const char (&stringLiteral)[N]);

    [[nodiscard]] const char* Data() const;

    [[nodiscard]] bool IsEmpty() const;
    [[nodiscard]] uint Size() const;

    [[nodiscard]] ConstIter_t cbegin() const;
    [[nodiscard]] ConstIter_t begin() const;
    [[nodiscard]] ConstIter_t cend() const;
    [[nodiscard]] ConstIter_t end() const;

private:
    const char* string_{};
    uint size_{};
};

//------------------------------------------------------------------------------
// String
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
bool String::IsEmpty() const
{
    return size_ == 0;
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

//------------------------------------------------------------------------------
// String view
//------------------------------------------------------------------------------
template<uint N>
constexpr StringView::StringView(const char (&stringLiteral)[N])
{
    size_ = N - 1;
    string_ = stringLiteral;
}

//------------------------------------------------------------------------------
bool StringView::IsEmpty() const
{
    return size_ == 0;
}

//------------------------------------------------------------------------------
uint StringView::Size() const
{
    return size_;
}

//------------------------------------------------------------------------------
const char* StringView::Data() const
{
    return string_;
}

//------------------------------------------------------------------------------
StringView::ConstIter_t StringView::cbegin() const
{
    return string_;
}

//------------------------------------------------------------------------------
StringView::ConstIter_t StringView::begin() const
{
    return cbegin();
}

//------------------------------------------------------------------------------
StringView::ConstIter_t StringView::cend() const
{
    return string_ + size_;
}

//------------------------------------------------------------------------------
StringView::ConstIter_t StringView::end() const
{
    return cend();
}

}
