#include "ParticleManager.h"

void ParticleManager::UpdateParticles(const std::vector<ParticleRenderData>& particles)
{
    m_activeParticles = particles;
}

void ParticleManager::AddParticle(const ParticleRenderData& particle)
{
    m_activeParticles.push_back(particle);
}

void ParticleManager::Clear()
{
    m_activeParticles.clear();
}