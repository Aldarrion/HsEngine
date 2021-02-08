#pragma once

//------------------------------------------------------------------------------
#ifdef NDEBUG
    #define HS_DEBUG 0
#else
    #define HS_DEBUG 1
#endif

//------------------------------------------------------------------------------
#ifndef HS_WINDOWS
    #define HS_WINDOWS 0
#endif

#ifndef HS_LINUX
    #define HS_LINUX 0
#endif

//------------------------------------------------------------------------------
#ifndef HS_MSVC
    #define HS_MSVC 0
#endif

#ifndef HS_CLANG
    #define HS_CLANG 0
#endif

#ifndef HS_GCC
    #define HS_GCC 0
#endif
