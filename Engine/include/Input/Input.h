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

enum KeyCode
{
    KC_SPACE = ' ',
    KC_A = 'A',
    KC_W = 'W',
    KC_S = 'S',
    KC_D = 'D',
    KC_C = 'C',
    KC_Q = 'Q',
    KC_E = 'E',

    #if HS_WINDOWS
        KC_LSHIFT = VK_LSHIFT,
    #elif HS_LINUX
        KC_LSHIFT = GLFW_KEY_LEFT_SHIFT,
    #endif
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

    bool IsKeyDown(KeyCode keyCode) const;
    bool IsKeyUp(KeyCode keyCode) const;

    // Keys
    bool GetState(KeyCode keyCode) const;
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
