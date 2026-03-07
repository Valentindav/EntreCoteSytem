#pragma once
#include "Private/Systems/System.h"
#include <vector>

class UiTransformComponent;

class UiRenderSystem : public System
{
public:
    void AddComponent(Component* _comp) override;
    void Update() override;

private:
    std::vector<UiTransformComponent*> m_uiTransforms;
};