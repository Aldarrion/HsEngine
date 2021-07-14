#include "Input/Input.h"

#include "Engine.h"

#include "Render/Render.h"

#include "Common/Logging.h"

#if HS_WINDOWS
    #include "Platform/hs_Windows.h"
#endif

#include "GLFW/glfw3.h"

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

//------------------------------------------------------------------------------
//static void 

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
static void JoysticCallback(int joysticId, int event)
{
    if (event == GLFW_CONNECTED)
    {
        const char* name = glfwGetJoystickName(joysticId);
        LOG_DBG("Gamepad %d \"%s\" connected", joysticId, name);
    }
    else if (event == GLFW_DISCONNECTED)
    {
        const char* name = glfwGetJoystickName(joysticId);
        LOG_DBG("Gamepad %d \"%s\" disconnected", joysticId, name);
    }
}

//------------------------------------------------------------------------------
RESULT Input::Init()
{
    glfwSetJoystickCallback(&JoysticCallback);

    for (int gamepadI = 0; gamepadI < GLFW_JOYSTICK_LAST; ++gamepadI)
    {
        if (glfwJoystickIsGamepad(gamepadI))
        {
            const char* name = glfwGetJoystickName(gamepadI);
            LOG_DBG("Gamepad %d \"%s\" connected", gamepadI, name);
        }
    }

    return R_OK;
}

#if HS_WINDOWS
    //------------------------------------------------------------------------------
    RESULT Input::InitWin32(HWND hwnd)
    {
        hwnd_ = hwnd;

        auto res = Init();

        return res;
    }
#elif HS_LINUX
    //------------------------------------------------------------------------------
    RESULT Input::InitLinux(GLFWwindow* window)
    {
        window_ = window;

        glfwSetKeyCallback(window_, &KeyCallback);

        auto res = Init();

        return res;
    }
#endif

//------------------------------------------------------------------------------
void Input::Update()
{
    #if HS_WINDOWS
        //glfwPollEvents();
    #endif

    for (int gamepadI = 0; gamepadI < GLFW_JOYSTICK_LAST; ++gamepadI)
    {
        memcpy(&previousGamepads_[gamepadI], &currentGamepads_[gamepadI], sizeof(GLFWgamepadstate));

        GLFWgamepadstate state;
        if (glfwJoystickIsGamepad(gamepadI) && glfwGetGamepadState(gamepadI, &state))
            currentGamepads_[gamepadI] = state;
        else
            currentGamepads_[gamepadI] = {};
    }

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
bool Input::IsKeyDown(KeyCode keyCode) const
{
    for (int i = 0; i < keysDown_.Count(); ++i)
    {
        if (keysDown_[i] == keyCode)
            return true;
    }

    return false;
}

//------------------------------------------------------------------------------
bool Input::IsKeyUp(KeyCode keyCode) const
{
    for (int i = 0; i < keysUp_.Count(); ++i)
    {
        if (keysUp_[i] == keyCode)
            return true;
    }

    return false;
}

//------------------------------------------------------------------------------
bool Input::GetState(KeyCode keyCode) const
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
bool Input::GetState(uint gamepad, int gamepadButton) const
{
    HS_ASSERT(gamepad < GLFW_JOYSTICK_LAST);
    HS_ASSERT(gamepadButton < GLFW_GAMEPAD_BUTTON_LAST);

    return currentGamepads_[gamepad].buttons[gamepadButton] == GLFW_PRESS;
}

//------------------------------------------------------------------------------
float Input::GetAxis(uint gamepad, int axis) const
{
    HS_ASSERT(gamepad < GLFW_JOYSTICK_LAST);
    HS_ASSERT(axis < GLFW_GAMEPAD_AXIS_LAST);

    float val = currentGamepads_[gamepad].axes[axis];
    if (val < 0 && val > -THUMBSTICK_DEADZONE)
        return 0;
    else if (val > 0 && val < THUMBSTICK_DEADZONE)
        return 0;

    return val;
}

//------------------------------------------------------------------------------
bool Input::IsButtonDown(MouseButton btn) const
{
    HS_ASSERT(btn < BTN_COUNT);

    return buttons_[btn] == ButtonState::Down;
}

//------------------------------------------------------------------------------
bool Input::IsButtonUp(MouseButton btn) const
{
    HS_ASSERT(btn < BTN_COUNT);

    return buttons_[btn] == ButtonState::Up;
}

//------------------------------------------------------------------------------
bool Input::IsButtonDown(uint gamepad, int gamepadButton) const
{
    HS_ASSERT(gamepad < GLFW_JOYSTICK_LAST);
    HS_ASSERT(gamepadButton < GLFW_GAMEPAD_BUTTON_LAST);

    return currentGamepads_[gamepad].buttons[gamepadButton] == GLFW_PRESS
        && previousGamepads_[gamepad].buttons[gamepadButton] == GLFW_RELEASE;
}

//------------------------------------------------------------------------------
bool Input::IsButtonUp(uint gamepad, int gamepadButton) const
{
    HS_ASSERT(gamepad < GLFW_JOYSTICK_LAST);
    HS_ASSERT(gamepadButton < GLFW_GAMEPAD_BUTTON_LAST);

    return currentGamepads_[gamepad].buttons[gamepadButton] == GLFW_RELEASE
        && previousGamepads_[gamepad].buttons[gamepadButton] == GLFW_PRESS;
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
    HS_ASSERT(button < BTN_COUNT);

    buttons_[button] = ButtonState::Down;
}

//------------------------------------------------------------------------------
void Input::ButtonUp(MouseButton button)
{
    HS_ASSERT(button < BTN_COUNT);

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
                HS_ASSERT(showCount < 0);
            #elif HS_LINUX
                glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            #endif
            CenterCursor();
        }
        else
        {
            HS_ASSERT(mouseMode_ == MouseMode::Absolute);
            #if HS_WINDOWS
                int showCount = ShowCursor(true);
                HS_ASSERT(showCount >= 0);
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

//------------------------------------------------------------------------------
bool Input::IsGamepadConnected(int gamepadId)
{
    return glfwJoystickIsGamepad(gamepadId);
}

}
