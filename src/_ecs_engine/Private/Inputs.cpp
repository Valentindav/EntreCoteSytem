#include "Public/Inputs.h"
#include <windowsx.h>

void Inputs::ProcessWindowsMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        // =========================
        // CLAVIER
        // =========================
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    {
        Keyboard::Key key = VKToKeyboardEnum(wParam);
        if (key != (Keyboard::Key)-1)
        {
            bool wasDown = (lParam & (1 << 30)) != 0;
            if (!wasDown)
            {
                OnKeyDown(key);
            }
        }
        break;
    }
    case WM_KEYUP:
    case WM_SYSKEYUP:
    {
        Keyboard::Key key = VKToKeyboardEnum(wParam);
        if (key != (Keyboard::Key)-1)
            OnKeyUp(key);
        break;
    }

    // =========================
    // SOURIS (MOUVEMENT ET MOLETTE)
    // =========================
    case WM_MOUSEMOVE:
        OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        break;

    case WM_MOUSEWHEEL:
    {
        float wheelDelta = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / WHEEL_DELTA;
        s_mouseWheel += wheelDelta;
        break;
    }


    // =========================
    // SOURIS (BOUTONS APPUYÉS)
    // =========================
    case WM_LBUTTONDOWN:
        OnMouseDown(VKToMouseEnum(VK_LBUTTON));
        break;
    case WM_RBUTTONDOWN:
        OnMouseDown(VKToMouseEnum(VK_RBUTTON));
        break;
    case WM_MBUTTONDOWN:
        OnMouseDown(VKToMouseEnum(VK_MBUTTON));
        break;
    case WM_XBUTTONDOWN:
    {
        int button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? VK_XBUTTON1 : VK_XBUTTON2;
        OnMouseDown(VKToMouseEnum(button));
        break;
    }

    // =========================
    // SOURIS (BOUTONS RELÂCHÉS)
    // =========================
    case WM_LBUTTONUP:
        OnMouseUp(VKToMouseEnum(VK_LBUTTON));
        break;
    case WM_RBUTTONUP:
        OnMouseUp(VKToMouseEnum(VK_RBUTTON));
        break;
    case WM_MBUTTONUP:
        OnMouseUp(VKToMouseEnum(VK_MBUTTON));
        break;
    case WM_XBUTTONUP:
    {
        int button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? VK_XBUTTON1 : VK_XBUTTON2;
        OnMouseUp(VKToMouseEnum(button));
        break;
    }
    }
}

/////////////////////////////////////
/// @brief init the input system.
/// @param take the window as param
/////////////////////////////////////
void Inputs::Initialize(HWND hwnd)
{
    if (hwnd != nullptr)
        s_windowHandle = hwnd;

    for (int i = 0; i < Keyboard::AMOUNT; i++)
        s_keyboardStates[i] = NONE;

    for (int i = 0; i < Mouse::AMOUNT; i++)
        s_mouseStates[i] = NONE;

    for (int i = 0; i < Gamepad::AMOUNT; i++)
        s_gamepadStates[i] = NONE;
}

//----------------------------------------------KEYBOARD----------------------------------------
/////////////////////////////////////
/// @brief manage the keyboard input
/// @param the key you want to press
/// @return the state of the concerned key
/// @note keyDown : only once when down
/// @note keyUp : only once when key released 
/// @note keyPressed : multiple time while key is pressed
/////////////////////////////////////
void Inputs::OnKeyDown(Keyboard::Key key)
{
    if (key < 0 || key >= Keyboard::AMOUNT) return;

    if (s_keyboardStates[key] == NONE || s_keyboardStates[key] == UP)
        s_keyboardStates[key] = DOWN;
}

void Inputs::OnKeyUp(Keyboard::Key key)
{
    if (key < 0 || key >= Keyboard::AMOUNT) return;

    s_keyboardStates[key] = UP;
}

bool Inputs::IsKeyDown(Keyboard::Key key)
{
    if (key < 0 || key >= Keyboard::AMOUNT) return false;
    return s_keyboardStates[key] == DOWN;
}

bool Inputs::IsKeyPressed(Keyboard::Key key)
{
    if (key < 0 || key >= Keyboard::AMOUNT) return false;
    return s_keyboardStates[key] == DOWN || s_keyboardStates[key] == PRESSED;
}

bool Inputs::IsKeyHeld(Keyboard::Key key)
{
    if (key < 0 || key >= Keyboard::AMOUNT) return false;
    return s_keyboardStates[key] == PRESSED;
}

bool Inputs::IsKeyUp(Keyboard::Key key)
{
    if (key < 0 || key >= Keyboard::AMOUNT) return false;
    return s_keyboardStates[key] == UP;
}

//-------------------------------------------MOUSE----------------------------------
/////////////////////////////////////
/// @brief manage mouse inputs.
/// @param the mouse button you want to press
/// @return the state of the button
/// @note same as for the keyboard input
/////////////////////////////////////
void Inputs::OnMouseDown(Mouse::Button button)
{
    if (button < 0 || button >= Mouse::AMOUNT) return;

    if (s_mouseStates[button] == NONE || s_mouseStates[button] == UP)
        s_mouseStates[button] = DOWN;
}

void Inputs::OnMouseUp(Mouse::Button button)
{
    if (button < 0 || button >= Mouse::AMOUNT) return;
    s_mouseStates[button] = UP;
}

bool Inputs::IsMouseDown(Mouse::Button button)
{
    if (button < 0 || button >= Mouse::AMOUNT) return false;
    return s_mouseStates[button] == DOWN;
}

bool Inputs::IsMousePressed(Mouse::Button button)
{
    if (button < 0 || button >= Mouse::AMOUNT) return false;
    return s_mouseStates[button] == DOWN || s_mouseStates[button] == PRESSED;
}

bool Inputs::IsMouseHeld(Mouse::Button button)
{
    if (button < 0 || button >= Mouse::AMOUNT) return false;
    return s_mouseStates[button] == PRESSED;
}

bool Inputs::IsMouseUp(Mouse::Button button)
{
    if (button < 0 || button >= Mouse::AMOUNT) return false;
    return s_mouseStates[button] == UP;
}

void Inputs::OnMouseMove(int x, int y)
{
    if (s_mouseLocked)
    {
        float dx = static_cast<float>(x - s_lockPosition.x);
        float dy = static_cast<float>(y - s_lockPosition.y);

        if (dx != 0.0f || dy != 0.0f)
        {
            s_mouseDelta.x += dx;
            s_mouseDelta.y += dy;

            if (s_windowHandle)
            {
                POINT centerScreen = s_lockPosition;
                ClientToScreen(s_windowHandle, &centerScreen);
                SetCursorPos(centerScreen.x, centerScreen.y);
            }
        }
    }
    else
    {
        s_mousePosition.x = static_cast<float>(x);
        s_mousePosition.y = static_cast<float>(y);
    }
}

void Inputs::OnMouseWheel(float delta)
{
    s_mouseWheel = delta;
}

/////////////////////////////////////
/// @brief Manage other mouse information.
/// @return those information
/// @note such as mousePos,delta,mouseWheel,cursor
/////////////////////////////////////

DirectX::XMFLOAT2 Inputs::GetMousePosition()
{
    return s_mousePosition;
}

DirectX::XMFLOAT2 Inputs::GetMouseDelta()
{
    return s_mouseDelta;
}

float Inputs::GetMouseWheel()
{
    return s_mouseWheel;
}

void Inputs::SetMousePosition(DirectX::XMFLOAT2 const& coordinates)
{
    if (s_windowHandle != nullptr)
    {
        POINT p{ static_cast<LONG>(coordinates.x), static_cast<LONG>(coordinates.y) };
        ClientToScreen(s_windowHandle, &p);
        SetCursorPos(p.x, p.y);
        s_mousePosition = coordinates;
    }
}

void Inputs::LockMouse(bool lock)
{
    if (s_windowHandle == nullptr)
        return;

    s_mouseLocked = lock;

    if (lock)
    {
        RECT rect;
        GetClientRect(s_windowHandle, &rect);

        POINT topLeft = { rect.left, rect.top };
        POINT bottomRight = { rect.right, rect.bottom };

        ClientToScreen(s_windowHandle, &topLeft);
        ClientToScreen(s_windowHandle, &bottomRight);

        RECT clipRect = { topLeft.x, topLeft.y, bottomRight.x, bottomRight.y };
        ClipCursor(&clipRect);

        while (::ShowCursor(FALSE) >= 0);
        s_cursorVisible = false;

        s_lockPosition.x = (rect.right - rect.left) / 2;
        s_lockPosition.y = (rect.bottom - rect.top) / 2;
    }
    else
    {
        ClipCursor(nullptr);

        while (::ShowCursor(TRUE) < 0);
        s_cursorVisible = true;
    }
}

bool Inputs::IsMouseLocked()
{
    return s_mouseLocked;
}


void Inputs::ShowCursor(bool show)
{
    if (show && !s_cursorVisible)
    {
        while (::ShowCursor(TRUE) < 0);
        s_cursorVisible = true;
    }
    else if (!show)
    {
        while (::ShowCursor(FALSE) >= 0);
        s_cursorVisible = false;
    }
}

bool Inputs::IsCursorVisible()
{
    return s_cursorVisible;
}

//--------------------------------------------REST---------------------------------

void Inputs::UpdateDeltas()
{
    if (!s_mouseLocked)
    {
        s_mouseDelta.x = s_mousePosition.x - s_mousePositionPrevious.x;
        s_mouseDelta.y = s_mousePosition.y - s_mousePositionPrevious.y;
        s_mousePositionPrevious = s_mousePosition;
    }
}

void Inputs::Update()
{
    if (!s_mouseLocked)
    {
        s_mouseDelta.x = s_mousePosition.x - s_mousePositionPrevious.x;
        s_mouseDelta.y = s_mousePosition.y - s_mousePositionPrevious.y;
        s_mousePositionPrevious = s_mousePosition;
    }

    s_mouseWheel = 0.0f;

    for (int i = 0; i < Keyboard::AMOUNT; i++)
    {
        if (s_keyboardStates[i] == DOWN) s_keyboardStates[i] = PRESSED;
        else if (s_keyboardStates[i] == UP) s_keyboardStates[i] = NONE;
    }

    for (int i = 0; i < Mouse::AMOUNT; i++)
    {
        if (s_mouseStates[i] == DOWN) s_mouseStates[i] = PRESSED;
        else if (s_mouseStates[i] == UP) s_mouseStates[i] = NONE;
    }

    for (int i = 0; i < Gamepad::AMOUNT; i++)
    {
        if (s_gamepadStates[i] == DOWN) s_gamepadStates[i] = PRESSED;
        else if (s_gamepadStates[i] == UP) s_gamepadStates[i] = NONE;
    }
}

void Inputs::EndFrameStates()
{
    if (s_mouseLocked)
    {
        s_mouseDelta.x = 0.0f;
        s_mouseDelta.y = 0.0f;
    }

    s_mouseWheel = 0.0f;

    for (int i = 0; i < Keyboard::AMOUNT; i++)
    {
        if (s_keyboardStates[i] == DOWN) s_keyboardStates[i] = PRESSED;
        else if (s_keyboardStates[i] == UP) s_keyboardStates[i] = NONE;
    }

    for (int i = 0; i < Mouse::AMOUNT; i++)
    {
        if (s_mouseStates[i] == DOWN) s_mouseStates[i] = PRESSED;
        else if (s_mouseStates[i] == UP) s_mouseStates[i] = NONE;
    }

    for (int i = 0; i < Gamepad::AMOUNT; i++)
    {
        if (s_gamepadStates[i] == DOWN) s_gamepadStates[i] = PRESSED;
        else if (s_gamepadStates[i] == UP) s_gamepadStates[i] = NONE;
    }
}
