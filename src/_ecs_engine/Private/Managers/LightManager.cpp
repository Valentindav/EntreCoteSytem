#include "LightManager.h"

void LightManager::UpdateLights(const std::vector<LightData>& lights)
{
    m_activeLights = lights;
}

void LightManager::AddLight(const LightData& light)
{
    m_activeLights.push_back(light);
}

void LightManager::Clear()
{
    m_activeLights.clear();
}