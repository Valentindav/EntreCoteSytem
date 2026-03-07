#pragma once
#include <cstdint>

struct ParticleRenderData
{
    uint32_t EmitterId;

    float originX, originY, originZ;
    float spawnRadius;
    float gravity;
    float speed;
    float spread;
    float minLife;
    float maxLife;
    float sizeStart;
    float sizeEnd;

    float colorStartR, colorStartG, colorStartB, colorStartA;
    float colorEndR, colorEndG, colorEndB, colorEndA;

    uint32_t maxParticles;
    float emitRate;
    float deltaAccum;
    bool enabled;
};