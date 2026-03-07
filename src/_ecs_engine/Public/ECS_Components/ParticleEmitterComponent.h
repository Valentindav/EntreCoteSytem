#pragma once
#include "private/Component.h"

#include <string>
#include <cstdint>

struct ParticleEmitterComponent : public Component
{
    float spawnRadius = 0.0f;
    float gravity = 9.81f;
    float speed = 3.0f;
    float spread = 0.5f;
    float minLife = 1.0f;
    float maxLife = 3.0f;
    float sizeStart = 0.08f;
    float sizeEnd = 0.02f;

    float colorStartR = 1.0f, colorStartG = 0.5f, colorStartB = 0.0f, colorStartA = 1.0f;
    float colorEndR = 1.0f, colorEndG = 0.0f, colorEndB = 0.0f, colorEndA = 0.0f;

    uint32_t maxParticles = 1024;
    float emitRate = 200.0f;
    bool enabled = true;

    float deltaAccum = 0.0f;

    virtual const ComponentType::Type GetType() override { return ComponentType::ParticleEmitter; }
};  