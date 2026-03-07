#pragma once
#include "Private/InputEnums.h"
#include <unordered_map>
#include <Windows.h> 
#include <Xinput.h>
#include <DirectXMath.h>

#ifndef XINPUT_GAMEPAD_DPAD_UP

#define XINPUT_GAMEPAD_DPAD_UP        0x0001
#define XINPUT_GAMEPAD_DPAD_DOWN      0x0002
#define XINPUT_GAMEPAD_DPAD_LEFT      0x0004
#define XINPUT_GAMEPAD_DPAD_RIGHT     0x0008
#define XINPUT_GAMEPAD_START          0x0010
#define XINPUT_GAMEPAD_BACK           0x0020
#define XINPUT_GAMEPAD_LEFT_THUMB     0x0040
#define XINPUT_GAMEPAD_RIGHT_THUMB    0x0080
#define XINPUT_GAMEPAD_LEFT_SHOULDER  0x0100
#define XINPUT_GAMEPAD_RIGHT_SHOULDER 0x0200
#define XINPUT_GAMEPAD_A              0x1000
#define XINPUT_GAMEPAD_B              0x2000
#define XINPUT_GAMEPAD_X              0x4000
#define XINPUT_GAMEPAD_Y              0x8000
#endif

inline Keyboard::Key VKToKeyboardEnum(WPARAM vk)
{
    switch (vk)
    {
    case 'A': return Keyboard::A;
    case 'B': return Keyboard::B;
    case 'C': return Keyboard::C;
    case 'D': return Keyboard::D;
    case 'E': return Keyboard::E;
    case 'F': return Keyboard::F;
    case 'G': return Keyboard::G;
    case 'H': return Keyboard::H;
    case 'I': return Keyboard::I;
    case 'J': return Keyboard::J;
    case 'K': return Keyboard::K;
    case 'L': return Keyboard::L;
    case 'M': return Keyboard::M;
    case 'N': return Keyboard::N;
    case 'O': return Keyboard::O;
    case 'P': return Keyboard::P;
    case 'Q': return Keyboard::Q;
    case 'R': return Keyboard::R;
    case 'S': return Keyboard::S;
    case 'T': return Keyboard::T;
    case 'U': return Keyboard::U;
    case 'V': return Keyboard::V;
    case 'W': return Keyboard::W;
    case 'X': return Keyboard::X;
    case 'Y': return Keyboard::Y;
    case 'Z': return Keyboard::Z;
    case '0': return Keyboard::_0;
    case '1': return Keyboard::_1;
    case '2': return Keyboard::_2;
    case '3': return Keyboard::_3;
    case '4': return Keyboard::_4;
    case '5': return Keyboard::_5;
    case '6': return Keyboard::_6;
    case '7': return Keyboard::_7;
    case '8': return Keyboard::_8;
    case '9': return Keyboard::_9;
    case VK_SPACE: return Keyboard::SPACE;
    case VK_ESCAPE: return Keyboard::ESCAPE;
    case VK_RETURN: return Keyboard::RETURN;
    case VK_BACK: return Keyboard::BACKSPACE;
    case VK_TAB: return Keyboard::TAB;
    case VK_LCONTROL: return Keyboard::LCTRL;
    case VK_RCONTROL: return Keyboard::RCTRL;
    case VK_LSHIFT: return Keyboard::LSHIFT;
    case VK_RSHIFT: return Keyboard::RSHIFT;
    case VK_LMENU: return Keyboard::LALT;
    case VK_RMENU: return Keyboard::RALT;
    case VK_LEFT: return Keyboard::LEFT;
    case VK_RIGHT: return Keyboard::RIGHT;
    case VK_UP: return Keyboard::UP;
    case VK_DOWN: return Keyboard::DOWN;
    case VK_F1: return Keyboard::F1;
    case VK_F2: return Keyboard::F2;
    case VK_F3: return Keyboard::F3;
    case VK_F4: return Keyboard::F4;
    case VK_F5: return Keyboard::F5;
    case VK_F6: return Keyboard::F6;
    case VK_F7: return Keyboard::F7;
    case VK_F8: return Keyboard::F8;
    case VK_F9: return Keyboard::F9;
    case VK_F10: return Keyboard::F10;
    case VK_F11: return Keyboard::F11;
    case VK_F12: return Keyboard::F12;
    case VK_NUMPAD0: return Keyboard::NUMPAD0;
    case VK_NUMPAD1: return Keyboard::NUMPAD1;
    case VK_NUMPAD2: return Keyboard::NUMPAD2;
    case VK_NUMPAD3: return Keyboard::NUMPAD3;
    case VK_NUMPAD4: return Keyboard::NUMPAD4;
    case VK_NUMPAD5: return Keyboard::NUMPAD5;
    case VK_NUMPAD6: return Keyboard::NUMPAD6;
    case VK_NUMPAD7: return Keyboard::NUMPAD7;
    case VK_NUMPAD8: return Keyboard::NUMPAD8;
    case VK_NUMPAD9: return Keyboard::NUMPAD9;
    case VK_INSERT: return Keyboard::INSERT;
    case VK_DELETE: return Keyboard::DELETE_;
    case VK_HOME: return Keyboard::HOME;
    case VK_END: return Keyboard::END;
    case VK_PRIOR: return Keyboard::PAGE_UP;
    case VK_NEXT: return Keyboard::PAGE_DOWN;
    default: return (Keyboard::Key)-1;
    }
}

inline Mouse::Button VKToMouseEnum(WPARAM vk)
{
    if (vk & MK_LBUTTON)   return Mouse::LEFT;
    if (vk & MK_RBUTTON)   return Mouse::RIGHT;
    if (vk & MK_MBUTTON)   return Mouse::MIDDLE;
#ifdef MK_XBUTTON1
    if (vk & MK_XBUTTON1)  return Mouse::EXTRA_1;
#endif
#ifdef MK_XBUTTON2
    if (vk & MK_XBUTTON2)  return Mouse::EXTRA_2;
#endif

    switch ((UINT)vk)
    {
    case VK_LBUTTON: return Mouse::LEFT;
    case VK_RBUTTON: return Mouse::RIGHT;
    case VK_MBUTTON: return Mouse::MIDDLE;
    case VK_XBUTTON1: return Mouse::EXTRA_1;
    case VK_XBUTTON2: return Mouse::EXTRA_2;
    default: return (Mouse::Button)-1;
    }
}

inline Gamepad::Button XInputToGamepadEnum(WORD buttonsMask)
{
    // Retourne le premier bouton détecté dans l'ordre courant.
    if (buttonsMask & XINPUT_GAMEPAD_A)             return Gamepad::A;
    if (buttonsMask & XINPUT_GAMEPAD_B)             return Gamepad::B;
    if (buttonsMask & XINPUT_GAMEPAD_X)             return Gamepad::X;
    if (buttonsMask & XINPUT_GAMEPAD_Y)             return Gamepad::Y;
    if (buttonsMask & XINPUT_GAMEPAD_LEFT_SHOULDER) return Gamepad::LEFT_SHOULDER;
    if (buttonsMask & XINPUT_GAMEPAD_RIGHT_SHOULDER)return Gamepad::RIGHT_SHOULDER;
    if (buttonsMask & XINPUT_GAMEPAD_BACK)          return Gamepad::BACK;
    if (buttonsMask & XINPUT_GAMEPAD_START)         return Gamepad::START;
    if (buttonsMask & XINPUT_GAMEPAD_LEFT_THUMB)    return Gamepad::LEFT_THUMB;
    if (buttonsMask & XINPUT_GAMEPAD_RIGHT_THUMB)   return Gamepad::RIGHT_THUMB;
    if (buttonsMask & XINPUT_GAMEPAD_DPAD_UP)       return Gamepad::D_PAD_UP;
    if (buttonsMask & XINPUT_GAMEPAD_DPAD_DOWN)     return Gamepad::D_PAD_DOWN;
    if (buttonsMask & XINPUT_GAMEPAD_DPAD_LEFT)     return Gamepad::D_PAD_LEFT;
    if (buttonsMask & XINPUT_GAMEPAD_DPAD_RIGHT)    return Gamepad::D_PAD_RIGHT;
    return (Gamepad::Button)-1;
}

inline Gamepad::Button VKToGamepadEnum(WPARAM vk)
{
    return XInputToGamepadEnum(static_cast<WORD>(vk));
}

class Inputs
{
public:
    enum States : __int8 {
        NONE = 0b00,
        PRESSED = 0b01,
        UP = 0b10,
        DOWN = 0b11
    };

	static std::unordered_map<__int8, __int32> s_keyboardMap;
	static std::unordered_map<__int8, __int32> s_mouseMap;

    inline static HWND s_windowHandle = nullptr;

    inline static bool s_mouseLocked = false;      
    inline static bool s_cursorVisible = true;    
    inline static POINT s_lockPosition = { 0, 0 };

	inline static States s_mouseStates[Mouse::AMOUNT];
	inline static States s_keyboardStates[Keyboard::AMOUNT];
	inline static States s_gamepadStates[Gamepad::AMOUNT];
	inline static float s_gamepadTriggers[2];
	inline static float s_gamepadSticks[4];
	inline static float s_gamepadVibration[2];

    static bool IsKeyDown(Keyboard::Key key);     
    static bool IsKeyPressed(Keyboard::Key key);  
    static bool IsKeyUp(Keyboard::Key key);       
    static bool IsKeyHeld(Keyboard::Key key);     

    static bool IsMouseDown(Mouse::Button button);
    static bool IsMousePressed(Mouse::Button button);
    static bool IsMouseUp(Mouse::Button button);
    static bool IsMouseHeld(Mouse::Button button);

    static void OnKeyDown(Keyboard::Key key);
    static void OnKeyUp(Keyboard::Key key);
    static void OnMouseDown(Mouse::Button button);
    static void OnMouseUp(Mouse::Button button);

    template<typename WindowType>
    static void BindToWindow(WindowType* window);

    static void UpdateDeltas();
    static void Update();
    static void EndFrameStates();

    static void Initialize(HWND hwnd = nullptr);

    static void ProcessWindowsMessage(UINT msg, WPARAM wParam, LPARAM lParam);

    static DirectX::XMFLOAT2 GetMousePosition();
    static void SetMousePosition(DirectX::XMFLOAT2 const& coordinates);


    inline static DirectX::XMFLOAT2 s_mousePosition = { 0.0f, 0.0f };
    inline static DirectX::XMFLOAT2 s_mousePositionPrevious = { 0.0f, 0.0f };
    inline static DirectX::XMFLOAT2 s_mouseDelta = { 0.0f, 0.0f };
    inline static float s_mouseWheel = 0.0f;

    static DirectX::XMFLOAT2 GetMouseDelta();
    static float GetMouseWheel();

    static void OnMouseMove(int x, int y);
    static void OnMouseWheel(float delta);

    static void LockMouse(bool lock);
    static bool IsMouseLocked();
    static void ShowCursor(bool show);
    static bool IsCursorVisible();
};

template<typename WindowType>
inline void Inputs::BindToWindow(WindowType* window)
{
    s_windowHandle = window->GetHwnd();
    Initialize(s_windowHandle);

    window->OnKeyDown = [](WPARAM wParam) {
        Keyboard::Key key = VKToKeyboardEnum(wParam);
        if (key != (Keyboard::Key)-1)
            Inputs::OnKeyDown(key);
        };

    window->OnKeyUp = [](WPARAM wParam) {
        Keyboard::Key key = VKToKeyboardEnum(wParam);
        if (key != (Keyboard::Key)-1)
            Inputs::OnKeyUp(key);
        };

    window->OnMouseDown = [](WPARAM btnState, int x, int y) {
        Mouse::Button key = VKToMouseEnum(btnState);
        if (key != (Mouse::Button)-1)
            Inputs::OnMouseDown(key);
        };
       

    window->OnMouseUp = [](WPARAM btnState, int x, int y) {
        Mouse::Button key = VKToMouseEnum(btnState);
        if (key != (Mouse::Button)-1)
            Inputs::OnMouseUp(key);
        };

    window->OnMouseMove = [](int x, int y) {
        Inputs::OnMouseMove(x, y);
        };

    window->OnMouseWheel = [](float delta) {
        Inputs::OnMouseWheel(delta);
        };
}