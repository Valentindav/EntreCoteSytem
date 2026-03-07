#pragma once
#include "Data/ParticleRenderData.h"
#include <vector>

class ParticleManager
{
public:
    ParticleManager() = default;
    ~ParticleManager() = default;

    ParticleManager(const ParticleManager&) = delete;
    ParticleManager& operator=(const ParticleManager&) = delete;

    void UpdateParticles(const std::vector<ParticleRenderData>& particles);
    void AddParticle(const ParticleRenderData& particle);

    void Clear();

    const std::vector<ParticleRenderData>& GetParticles() const { return m_activeParticles; }
    size_t GetParticleCount() const { return m_activeParticles.size(); }
    bool IsEmpty() const { return m_activeParticles.empty(); }

private:
    std::vector<ParticleRenderData> m_activeParticles;
};