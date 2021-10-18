#pragma once

#include "String/String.h"

#include "Common/Assert.h"

#include <cstdio>
#include <wchar.h>

namespace hs
{

//------------------------------------------------------------------------------
template<int LENGTH = 128>
struct WString
{
    wchar_t buffer_[LENGTH];
};

//------------------------------------------------------------------------------
template<int LENGTH = 128>
WString<LENGTH> MakeWString(StringView sw)
{
    WString<LENGTH> ws;

    int res = swprintf(ws.buffer_, LENGTH, L"%.*S", sw.Length(), sw.Data());
    HS_ASSERT(res <= LENGTH);

    return ws;
}

}
