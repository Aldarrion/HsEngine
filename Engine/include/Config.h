#pragma once

//------------------------------------------------------------------------------
#ifdef NDEBUG
    #define HS_DEBUG 0
    #define HS_RENDER_DEBUG 0
#else
    #define HS_DEBUG 1
    #define HS_RENDER_DEBUG 1
#endif

//------------------------------------------------------------------------------
#define HS_WINDOWS 0
#define HS_LINUX 0

#if defined(__linux__)
    #undef HS_LINUX
    #define HS_LINUX 1
#elif defined(_WIN32)
    #undef HS_WINDOWS
    #define HS_WINDOWS 1
#endif

//------------------------------------------------------------------------------
#define HS_MSVC 0
#define HS_CLANG 0
#define HS_GCC 0

#if defined(__clang__)
    #undef HS_CLANG
    #define HS_CLANG 1
#elif defined(_MSC_VER)
    #undef HS_MSVC
    #define HS_MSVC 1
#endif
