#include "Engine.h"

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
#endif

//------------------------------------------------------------------------------
bool Engine::IsWindowActive() const
{
    return isWindowActive_;
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
