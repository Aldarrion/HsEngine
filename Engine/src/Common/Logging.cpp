#include "Common/Logging.h"

#include "Common/Types.h"
#include "Common/Assert.h"

#include "Math/Math.h"

#if HS_WINDOWS
    #include "Platform/hs_Windows.h"
#endif

#include <cstring>
#include <cstdio>
#include <cstdarg>

namespace hs
{

//------------------------------------------------------------------------------
void Log(LogLevel level, const char* formatString, ...)
{
    va_list args;
    va_start(args, formatString);

    constexpr size_t buffSize = 2 << 13;
    char buffer[buffSize];

    const char* prefix = nullptr;

    switch (level)
    {
        case LogLevel::Info:
            prefix = "Info:    ";
            break;
        case LogLevel::Warning:
            prefix = "Warning: ";
            break;
        case LogLevel::Error:
            prefix = "Error:   ";
            break;
    }

    snprintf(buffer, buffSize - 1, "%s", prefix);

    const size_t prefixSize = strlen(prefix);
    const int len = vsnprintf(buffer + prefixSize, buffSize - 1 - prefixSize, formatString, args);
    HS_ASSERT(len >= 0 && "Logging failed, check the format string or exceeded length.");

    buffer[prefixSize + len] = '\n';
    buffer[prefixSize + len + 1] = '\0';

    #if HS_WINDOWS
        OutputDebugStringA(buffer);
    #else
        printf("%s", buffer);
    #endif
}

//------------------------------------------------------------------------------
void Mat44ToString(const Mat44& m, char* buff)
{
    uint offset = 0;
    for (int i = 0; i < 4; ++i)
    {
        offset += sprintf(buff + offset, "%.3f %.3f %.3f %.3f\n", m(i, 0), m(i, 1), m(i, 2), m(i, 3));
    }
}

//------------------------------------------------------------------------------
void LogMat44(const Mat44& m)
{
    static char c[512];
    Mat44ToString(m, c);
    Log(LogLevel::Info, "\n%s", c);
}

}
