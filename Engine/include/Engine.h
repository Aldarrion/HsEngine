#pragma once

#include "Config.h"

#include "Common/Enums.h"

#if HS_LINUX
    struct GLFWwindow;
#endif

namespace hs
{

//------------------------------------------------------------------------------
extern class Engine* g_Engine;

//------------------------------------------------------------------------------
RESULT CreateEngine();
void DestroyEngine();

//------------------------------------------------------------------------------
class Engine
{
public:
    ~Engine() = default;

    #if HS_WINDOWS
        RESULT InitWin32();
    #elif HS_LINUX
        RESULT InitLinux(GLFWwindow* window);
    #endif

    bool IsWindowActive() const;
    void SetWindowActive(bool isActive);
    float GetDTime() const;

    void Update(float dTime);

private:
    #if HS_WINDOWS
    #elif HS_LINUX
        GLFWwindow* window_{};
    #endif

    float dTime_{};
    bool isWindowActive_{};
};

}
