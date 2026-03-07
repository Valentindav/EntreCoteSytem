#pragma once
#include "Private/Systems/System.h"
#include <vector>

class UiButtonComponent;

class ButtonSystem : public System
{
public:
    void AddComponent(Component* _comp) override;
    void Update() override;

private:
    std::vector<UiButtonComponent*> m_buttons;
};