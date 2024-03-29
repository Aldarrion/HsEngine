#pragma once

#include "Common/Enums.h"

namespace hs
{

//------------------------------------------------------------------------------
extern class GameBase* g_GameBase;

//------------------------------------------------------------------------------
RESULT CreateGame();
void DestroyGame();

//------------------------------------------------------------------------------
class GameBase
{
public:
    virtual ~GameBase() = default;

    virtual RESULT Init() = 0;
    virtual RESULT OnWindowResized() = 0;
    virtual void Free() = 0;
    virtual void Update() = 0;
};

}
