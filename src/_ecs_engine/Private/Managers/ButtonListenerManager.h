#pragma once
#include <map>
#include <string>
#include <windows.h>

class ButtonListenerManager
{
public:
    inline static std::map<std::string, void(*)()> s_buttonListenersMap = {};

    inline static std::string s_currentInput = "";
    inline static bool s_inputComplete = false;
    inline static bool s_inputActive = false;

    inline static bool s_keyWasPressed[256] = { false };

    static void Init();
    static void RegisterButtonListener(const std::string& name, void (*l)());
    static void (*GetButtonListener(const std::string& name))();
};