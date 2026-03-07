#include "Private/Systems/ParticleSystem.h"

#include "Public/ECS_Components/ParticleEmitterComponent.h"
#include "Public/ECS_Components/TransformComponent.h"
#include "Public/ECS.h"
#include "Public/Entity.h"

#include "Private/Managers/ParticleManager.h"
#include "Private/EngineCore.h"

void ParticleSystem::AddComponent(Component* _comp)
{
    if (_comp->GetType() == ComponentType::ParticleEmitter)
    {
        m_emitters.push_back(static_cast<ParticleEmitterComponent*>(_comp));
    }
}

void ParticleSystem::Update()
{
    m_activeParticles.clear();

    // On récupère le temps depuis l'Engine
    float deltaTime = ECS_ENGINE->GetTimer().DeltaTime();

    for (ParticleEmitterComponent* emitter : m_emitters)
    {
        if (!emitter->enabled) continue;

        Entity* owner = emitter->GetOwner();
        TransformComponent* transform = &owner->transform;

        emitter->deltaAccum += deltaTime;

        ParticleRenderData data;

        data.EmitterId = owner->GetId();

        DirectX::XMFLOAT3 pos = transform->GetWorldPosition();
        data.originX = pos.x;
        data.originY = pos.y;
        data.originZ = pos.z;

        data.spawnRadius = emitter->spawnRadius;
        data.gravity = emitter->gravity;
        data.speed = emitter->speed;
        data.spread = emitter->spread;
        data.minLife = emitter->minLife;
        data.maxLife = emitter->maxLife;
        data.sizeStart = emitter->sizeStart;
        data.sizeEnd = emitter->sizeEnd;

        data.colorStartR = emitter->colorStartR;
        data.colorStartG = emitter->colorStartG;
        data.colorStartB = emitter->colorStartB;
        data.colorStartA = emitter->colorStartA;

        data.colorEndR = emitter->colorEndR;
        data.colorEndG = emitter->colorEndG;
        data.colorEndB = emitter->colorEndB;
        data.colorEndA = emitter->colorEndA;

        data.maxParticles = emitter->maxParticles;
        data.emitRate = emitter->emitRate;
        data.deltaAccum = emitter->deltaAccum;
        data.enabled = emitter->enabled;

        m_activeParticles.push_back(data);
    }
    m_emitters.clear();

    ParticleManager* particleManager = ECS_ENGINE->GetParticleManager();
    if (particleManager)
    {
        particleManager->UpdateParticles(m_activeParticles);
    }
}