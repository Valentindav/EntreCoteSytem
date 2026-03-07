#pragma once
#include "Private/Systems/System.h"

#include <vector>
#include <DirectXMath.h>
#include <Data/ParticleRenderData.h>

class ParticleEmitterComponent;

class ParticleSystem : public System
{
public:
    ParticleSystem() = default;
    ~ParticleSystem() = default;

    virtual void AddComponent(Component* _comp) override;
    virtual void Update() override;

private:
    std::vector<ParticleEmitterComponent*> m_emitters;
    std::vector<ParticleRenderData> m_activeParticles;
};