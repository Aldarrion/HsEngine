#pragma once
#include "Config.h"

#ifndef WINDOWS_LEAN_AND_MEAN
    #define WINDOWS_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
    #define NOMINMAX
#endif

#include <windows.h>

#ifdef near
    #undef near
#endif

#ifdef far
    #undef far
#endif

#ifdef DrawText
    #undef DrawText
#endif

