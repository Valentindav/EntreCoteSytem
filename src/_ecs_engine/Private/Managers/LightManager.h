#pragma once
#include "Data/LightData.h"
#include <vector>
#include <DirectXMath.h>

// Types de lumières
enum class LightTypeEnum { Directional = 0, Point = 1, Spot = 2 };

class LightManager
{
public:
    LightManager() = default;
    ~LightManager() = default;

    LightManager(const LightManager&) = delete;
    LightManager& operator=(const LightManager&) = delete;

    void UpdateLights(const std::vector<LightData>& lights);
    void AddLight(const LightData& light);

    void Clear();

    const std::vector<LightData>& GetLights() const { return m_activeLights; }
    size_t GetLightCount() const { return m_activeLights.size(); }
    bool IsEmpty() const { return m_activeLights.empty(); }

private:
    std::vector<LightData> m_activeLights;
};