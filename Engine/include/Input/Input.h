#pragma once

#include "Config.h"
#include "Common/Types.h"
#include "Common/Enums.h"
#include "Containers/Array.h"

#include "Math/hs_Math.h"

#if HS_WINDOWS
    #include "Platform/hs_Windows.h"
#endif

struct GLFWwindow;

namespace hs
{

//------------------------------------------------------------------------------
extern class Input* g_Input;

//------------------------------------------------------------------------------
RESULT CreateInput();
void DestroyInput();

//------------------------------------------------------------------------------
enum MouseButton
{
    BTN_LEFT,
    BTN_RIGHT,
    BTN_MIDDLE,
    BTN_COUNT
};

//------------------------------------------------------------------------------
enum class MouseMode
{
    Absolute,
    Relative,
};

//------------------------------------------------------------------------------
template<class KeyT, class DataT>
struct Pair
{
    KeyT    Key;
    DataT   Data;
};

//------------------------------------------------------------------------------
class Input
{
public:
    #if HS_WINDOWS
        RESULT InitWin32(HWND hwnd);
    #elif HS_LINUX
        RESULT InitLinux(GLFWwindow* window);
    #endif

    void Update();
    void EndFrame();

    Vec2 GetMousePos() const;

    bool IsKeyDown(int keyCode) const;
    bool IsKeyUp(int keyCode) const;

    // Keys
    bool GetState(int keyCode) const;
    bool GetState(MouseButton button) const;

    void KeyDown(int key);
    void KeyUp(int key);

    bool IsButtonDown(MouseButton button) const;
    bool IsButtonUp(MouseButton button) const;

    void ButtonDown(MouseButton button);
    void ButtonUp(MouseButton button);

    void SetMouseMode(MouseMode mode);

    Vec2 GetMouseDelta() const;

private:
    #if HS_WINDOWS
        HWND hwnd_;
    #elif HS_LINUX
        GLFWwindow* window_;
    #endif

    enum class ButtonState
    {
        None,
        Down,
        Up
    };

    ButtonState buttons_[BTN_COUNT]{};

    Array<int> keysDown_;
    Array<int> keysUp_;

    MouseMode mouseMode_{};
    Vec2 mouseDelta_{};

    void CenterCursor();
};

}
