#pragma once

#if HS_WINDOWS
    #include "Platform/hs_Windows.h"
#endif

namespace hs
{

#if HS_WINDOWS
    //------------------------------------------------------------------------------
    int EngineMainWin32(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int showCmd);
#elif HS_LINUX
    //------------------------------------------------------------------------------
    int EngineMainLinux(int argc, char** argv);
#endif

}
