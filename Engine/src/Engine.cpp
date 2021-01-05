#include "Engine.h"

#if HS_LINUX
    #include "GLFW/glfw3.h"
#endif

namespace hs
{

//------------------------------------------------------------------------------
Engine* g_Engine;

//------------------------------------------------------------------------------
class GameBase* g_GameBase;

//------------------------------------------------------------------------------
RESULT CreateEngine()
{
    g_Engine = new Engine();

    return R_OK;
}

//------------------------------------------------------------------------------
void DestroyEngine()
{
    delete g_Engine;
}

#if HS_WINDOWS
    //------------------------------------------------------------------------------
    RESULT Engine::InitWin32()
    {
        return R_OK;
    }
#elif HS_LINUX
    //------------------------------------------------------------------------------
    RESULT Engine::InitLinux(GLFWwindow* window)
    {
        window_ = window;
        return R_OK;
    }
#endif

//------------------------------------------------------------------------------
bool Engine::IsWindowActive() const
{
    #if HS_WINDOWS
        return isWindowActive_;
    #elif HS_LINUX
        return glfwGetWindowAttrib(window_, GLFW_FOCUSED) != 0;
    #endif
}

//------------------------------------------------------------------------------
void Engine::SetWindowActive(bool isActive)
{
    isWindowActive_ = isActive;
}

//------------------------------------------------------------------------------
void Engine::Update(float dTime)
{
    dTime_ = dTime;
}

//------------------------------------------------------------------------------
float Engine::GetDTime() const
{
    return dTime_;
}

}
