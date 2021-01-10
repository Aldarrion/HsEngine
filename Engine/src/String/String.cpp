#include "String/String.h"

#include "Common/Logging.h"

#include <cstdio>
#include <cstdarg>

namespace hs
{

//------------------------------------------------------------------------------
String String::FromFormat(uint maxLen, const char* formatString, ...)
{
    String s(maxLen);

    va_list args;
    va_start(args, formatString);

    int len = vsnprintf(s.Data(), maxLen, formatString, args);
    if (len < 0)
    {
        LOG_ERR("Failed to format string, max len possibly exceeded");
        len = 0;
    }

    s.size_ = len;
    return s;
}

//------------------------------------------------------------------------------
String::String(uint reservedCapacity)
    : capacity_(reservedCapacity)
    , size_(0)
{
    string_ = (char*)malloc(reservedCapacity);
}

//------------------------------------------------------------------------------
String::~String()
{
    size_ = 0;
    capacity_ = 0;
    free(string_);
}

//------------------------------------------------------------------------------
String::String(const String& other)
    : capacity_(other.capacity_)
    , size_(other.size_)
{
    string_ = (char*)malloc(capacity_);
    memcpy(string_, other.string_, size_);
}

//------------------------------------------------------------------------------
String& String::operator=(const String& other)
{
    if (this == &other)
        return *this;

    if (capacity_ != other.capacity_)
        string_ = (char*)realloc(string_, other.capacity_);

    capacity_ = other.capacity_;
    size_ = other.size_;
    memcpy(string_, other.string_, size_);

    return *this;
}

//------------------------------------------------------------------------------
String::String(String&& other)
    : string_(other.string_)
    , capacity_(other.capacity_)
    , size_(other.size_)
{
    other.string_ = nullptr;
    other.capacity_ = 0;
    other.size_ = 0;
}

//------------------------------------------------------------------------------
String& String::operator=(String&& other)
{
    if (this == &other)
        return *this;

    free(string_);
    string_ = other.string_;
    capacity_ = other.capacity_;
    size_ = other.size_;

    other.string_ = nullptr;
    other.capacity_ = 0;
    other.size_ = 0;

    return *this;
}

}
