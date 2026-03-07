#include "Private/Managers/ButtonListenerManager.h"

void ButtonListenerManager::Init()
{
}

void ButtonListenerManager::RegisterButtonListener(const std::string& name, void (*l)())
{
    s_buttonListenersMap[name] = l;
}

void(*ButtonListenerManager::GetButtonListener(const std::string& name))()
{
    auto it = s_buttonListenersMap.find(name);
    if (it != s_buttonListenersMap.end())
    {
        return it->second;
    }
    return nullptr;
}
