#pragma once

#if HS_WINDOWS
    #include "Platform/hs_Windows.h"
#endif

namespace hs
{

#if HS_WINDOWS
    //------------------------------------------------------------------------------
    int EngineMainWin32(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, const char* gameName);
#elif HS_LINUX
    //------------------------------------------------------------------------------
    int EngineMainLinux(int argc, char** argv, const char* gameName);
#endif

}
