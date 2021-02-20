#pragma once

#include "Common/Types.h"

#include <cstdlib>
#include <cstring>

namespace hs
{

class String;

//------------------------------------------------------------------------------
template<class StringT>
bool StringEquals(const StringT& a, const StringT& b)
{
    if (a.Length() != b.Length())
        return false;

    for (int i = 0, len = a.Length(); i < len; ++i)
    {
        if (a.Data()[i] != b.Data()[i])
            return false;
    }

    return true;
}

//------------------------------------------------------------------------------
class StringView
{
public:
    using Iter_t = const char*;
    using ConstIter_t = Iter_t;

    StringView() = default;
    explicit StringView(const String& string);
    explicit StringView(const char* cstr);

    //template<uint N>
    //explicit constexpr StringView(const char (&stringLiteral)[N]);


    [[nodiscard]] const char* Data() const;

    [[nodiscard]] bool IsEmpty() const;
    [[nodiscard]] uint Size() const;
    [[nodiscard]] uint Length() const;

    [[nodiscard]] ConstIter_t cbegin() const;
    [[nodiscard]] ConstIter_t begin() const;
    [[nodiscard]] ConstIter_t cend() const;
    [[nodiscard]] ConstIter_t end() const;

private:
    const char* string_{};
    uint size_{};
};

//------------------------------------------------------------------------------
class String
{
public:
    using Iter_t = char*;
    using ConstIter_t = const Iter_t;

    static String FromFormat(uint maxLen, const char* formatString, ...);

    String() = default;
    explicit String(uint reservedCapacity);
    explicit String(StringView strView);
    explicit String(const char* cstr);

    //template<uint N>
    //explicit constexpr String(const char (&stringLiteral)[N]);


    ~String();

    String(const String& other);
    String& operator=(const String& other);
    String(String&& other);
    String& operator=(String&& other);

    [[nodiscard]] const char* Data() const;
    [[nodiscard]] char* Data();

    [[nodiscard]] bool IsEmpty() const;
    [[nodiscard]] uint Size() const;
    [[nodiscard]] uint Length() const;

    void Append(StringView toAppend);

    [[nodiscard]] bool operator==(const String& other);

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
// String
//------------------------------------------------------------------------------
inline String::String(const char* cstr)
{
    capacity_ = strlen(cstr);
    string_ = (char*)malloc(capacity_);
    size_ = capacity_;

    memcpy(string_, cstr, capacity_);
}
//template<uint N>
//constexpr String::String(const char (&stringLiteral)[N])
//{
//    capacity_ = N - 1;
//    string_ = (char*)malloc(capacity_);
//    size_ = capacity_;
//
//    memcpy(string_, stringLiteral, capacity_);
//}

//------------------------------------------------------------------------------
inline bool String::IsEmpty() const
{
    return size_ == 0;
}

//------------------------------------------------------------------------------
inline uint String::Size() const
{
    return size_;
}

inline uint String::Length() const
{
    return size_;
}

//------------------------------------------------------------------------------
inline const char* String::Data() const
{
    return string_;
}

//------------------------------------------------------------------------------
inline char* String::Data()
{
    return string_;
}

//------------------------------------------------------------------------------
inline bool String::operator==(const String& other)
{
    const bool res = StringEquals(*this, other);
    return res;
}

//------------------------------------------------------------------------------
inline String::ConstIter_t String::cbegin() const
{
    return string_;
}

//------------------------------------------------------------------------------
inline String::ConstIter_t String::begin() const
{
    return cbegin();
}

//------------------------------------------------------------------------------
inline String::Iter_t String::begin()
{
    return string_;
}

//------------------------------------------------------------------------------
inline String::ConstIter_t String::cend() const
{
    return string_ + size_;
}

//------------------------------------------------------------------------------
inline String::ConstIter_t String::end() const
{
    return cend();
}

//------------------------------------------------------------------------------
inline String::Iter_t String::end()
{
    return string_ + size_;
}

//------------------------------------------------------------------------------
// String view
//------------------------------------------------------------------------------
inline StringView::StringView(const String& string)
    : string_(string.Data())
    , size_(string.Size())
{
}

//------------------------------------------------------------------------------
inline StringView::StringView(const char* cstr)
    : string_(cstr)
    , size_(strlen(cstr))
{
}
//template<uint N>
//constexpr StringView::StringView(const char (&stringLiteral)[N])
//{
//    size_ = N - 1;
//    string_ = stringLiteral;
//}

//------------------------------------------------------------------------------
inline bool StringView::IsEmpty() const
{
    return size_ == 0;
}

//------------------------------------------------------------------------------
inline uint StringView::Size() const
{
    return size_;
}

//------------------------------------------------------------------------------
inline uint StringView::Length() const
{
    return size_;
}

//------------------------------------------------------------------------------
inline const char* StringView::Data() const
{
    return string_;
}

//------------------------------------------------------------------------------
inline StringView::ConstIter_t StringView::cbegin() const
{
    return string_;
}

//------------------------------------------------------------------------------
inline StringView::ConstIter_t StringView::begin() const
{
    return cbegin();
}

//------------------------------------------------------------------------------
inline StringView::ConstIter_t StringView::cend() const
{
    return string_ + size_;
}

//------------------------------------------------------------------------------
inline StringView::ConstIter_t StringView::end() const
{
    return cend();
}

}
