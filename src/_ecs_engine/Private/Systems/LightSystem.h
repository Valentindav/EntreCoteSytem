#pragma once
#include "Private/Systems/System.h"

#include <Data/LightData.h>
#include <vector>
#include <DirectXMath.h>

class LightComponent;

class LightSystem : public System
{
public:
    LightSystem() = default;
    ~LightSystem() = default;

    virtual void AddComponent(Component* _comp) override;
    virtual void Update() override;

private:
    std::vector<LightComponent*> m_lights;
    std::vector<LightData> m_activeLights;
};