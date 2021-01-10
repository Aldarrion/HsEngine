#include "Input/Input.h"

#include "Engine.h"

#include "Render/Render.h"

#include "Common/Logging.h"

#if HS_WINDOWS
    #include "Platform/hs_Windows.h"
#elif HS_LINUX
    #include "GLFW/glfw3.h"
#endif

namespace hs
{

//------------------------------------------------------------------------------
Input* g_Input{};

//------------------------------------------------------------------------------
RESULT CreateInput()
{
    g_Input = new Input();

    return R_OK;
}

//------------------------------------------------------------------------------
void DestroyInput()
{
    delete g_Input;
}

#if HS_WINDOWS
    //------------------------------------------------------------------------------
    RESULT Input::InitWin32(HWND hwnd)
    {
        hwnd_ = hwnd;

        return R_OK;
    }
#elif HS_LINUX
    //------------------------------------------------------------------------------
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (action == GLFW_PRESS)
        {
            g_Input->KeyDown(key);
        }
        else if (action == GLFW_RELEASE)
        {
            g_Input->KeyUp(key);
        }
    }

    //------------------------------------------------------------------------------
    RESULT Input::InitLinux(GLFWwindow* window)
    {
        window_ = window;

        glfwSetKeyCallback(window_, &KeyCallback);

        return R_OK;
    }
#endif

//------------------------------------------------------------------------------
void Input::Update()
{
    if (mouseMode_ == MouseMode::Relative)
    {
        Vec2 mousePos = GetMousePos();
        mouseDelta_ = mousePos - Vec2{ g_Render->GetWidth() / 2.0f, g_Render->GetHeight() / 2.0f };

        CenterCursor();
    }
}

//------------------------------------------------------------------------------
void Input::EndFrame()
{
    memset(&buttons_, 0, sizeof(buttons_));
    keysDown_.Clear();
    keysUp_.Clear();
    mouseDelta_ = {};
}

//------------------------------------------------------------------------------
Vec2 Input::GetMousePos() const
{
    #if HS_WINDOWS
        POINT cursorPos;
        if (GetCursorPos(&cursorPos) == 0)
        {
            Log(LogLevel::Error, "Could not retrieve the cursor position, error: %d", GetLastError());
        }
        else
        {
            if (!ScreenToClient(hwnd_, &cursorPos))
            {
                Log(LogLevel::Error, "Could not cast cursor pos to client pos, error: %d", GetLastError());
            }
            else
            {
                return Vec2{ (float)cursorPos.x, (float)cursorPos.y };
            }
        }
    #elif HS_LINUX
        double xpos, ypos;
        glfwGetCursorPos(window_, &xpos, &ypos);
        return Vec2(static_cast<float>(xpos), static_cast<float>(ypos));
    #endif

    return Vec2{};
}

//------------------------------------------------------------------------------
bool Input::IsKeyDown(int keyCode) const
{
    for (int i = 0; i < keysDown_.Count(); ++i)
    {
        if (keysDown_[i] == keyCode)
            return true;
    }

    return false;
}

//------------------------------------------------------------------------------
bool Input::IsKeyUp(int keyCode) const
{
    for (int i = 0; i < keysUp_.Count(); ++i)
    {
        if (keysUp_[i] == keyCode)
            return true;
    }

    return false;
}

//------------------------------------------------------------------------------
bool Input::GetState(int keyCode) const
{
    if (!g_Engine->IsWindowActive())
        return false;
    #if HS_WINDOWS
        return (GetKeyState(keyCode) & 0x8000) != 0;
    #else
        auto lastState = glfwGetKey(window_, keyCode);
        return lastState == GLFW_PRESS;
    #endif
}

//------------------------------------------------------------------------------
bool Input::GetState(MouseButton button) const
{
    if (!g_Engine->IsWindowActive())
        return false;
    #if HS_WINDOWS
        // TODO this is hacky (button + 1) but we will use GLFW for everything eventually anyways.
        return (GetKeyState(button + 1) & 0x8000) != 0;
    #else
        auto lastState = glfwGetMouseButton(window_, button);
        return lastState == GLFW_PRESS;
    #endif
}

//------------------------------------------------------------------------------
bool Input::IsButtonDown(MouseButton btn) const
{
    hs_assert(btn < BTN_COUNT);

    return buttons_[btn] == ButtonState::Down;
}

//------------------------------------------------------------------------------
bool Input::IsButtonUp(MouseButton btn) const
{
    hs_assert(btn < BTN_COUNT);

    return buttons_[btn] == ButtonState::Up;
}

//------------------------------------------------------------------------------
void Input::KeyDown(int key)
{
    keysDown_.Add(key);
}

//------------------------------------------------------------------------------
void Input::KeyUp(int key)
{
    keysUp_.Add(key);
}

//------------------------------------------------------------------------------
void Input::ButtonDown(MouseButton button)
{
    hs_assert(button < BTN_COUNT);

    buttons_[button] = ButtonState::Down;
}

//------------------------------------------------------------------------------
void Input::ButtonUp(MouseButton button)
{
    hs_assert(button < BTN_COUNT);

    buttons_[button] = ButtonState::Up;
}

//------------------------------------------------------------------------------
void Input::CenterCursor()
{
    #if HS_WINDOWS
        POINT cursorPos { (int)g_Render->GetWidth() / 2, (int)g_Render->GetHeight() / 2 };
        if (ClientToScreen(hwnd_, &cursorPos) == 0)
        {
            Log(LogLevel::Error, "Could not retrieve the window position, error: %d", GetLastError());
        }
        else
        {
            if (SetCursorPos(cursorPos.x, cursorPos.y) == 0)
                Log(LogLevel::Error, "Could not set the cursor position, error: %d", GetLastError());
        }
    #elif HS_LINUX
        glfwSetCursorPos(window_, (int)g_Render->GetWidth() / 2, (int)g_Render->GetHeight() / 2);
    #endif
}

//------------------------------------------------------------------------------
void Input::SetMouseMode(MouseMode mode)
{
    if (mouseMode_ != mode)
    {
        mouseMode_ = mode;
        if (mouseMode_ == MouseMode::Relative)
        {
            #if HS_WINDOWS
                int showCount = ShowCursor(false);
                hs_assert(showCount < 0);
            #elif HS_LINUX
                glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            #endif
            CenterCursor();
        }
        else
        {
            hs_assert(mouseMode_ == MouseMode::Absolute);
            #if HS_WINDOWS
                int showCount = ShowCursor(true);
                hs_assert(showCount >= 0);
            #elif HS_LINUX
                glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            #endif
        }
    }
}

//------------------------------------------------------------------------------
Vec2 Input::GetMouseDelta() const
{
    return mouseDelta_;
}


}
