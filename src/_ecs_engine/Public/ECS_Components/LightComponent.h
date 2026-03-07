#pragma once
#include "Private/Component.h"
#include <DirectXMath.h>

using namespace DirectX;

enum class LightType {
    Directional,
    Point,
    Spot
};

class LightComponent : public Component
{
public:
    LightType m_type;

    XMFLOAT3 m_color = { 1.0f, 1.0f, 1.0f };
    float m_intensity = 1.0f;

    union {
        // Données pour une Directional Light
        struct {
            XMFLOAT3 m_dirDirection;
        };

        // Données pour une Point Light
        struct {
            float m_pointRange;
            XMFLOAT3 m_pointAttenuation; // x: Constant, y: Linear, z: Quadratic
        };

        // Données pour une Spot Light
        struct {
            XMFLOAT3 m_spotDirection;
            float m_spotRange;
            XMFLOAT3 m_spotAttenuation;
            float m_spotInnerAngle;
            float m_spotOuterAngle;
        };
    };

    LightComponent() : m_type(LightType::Directional) {
        m_dirDirection = { 0.0f, -1.0f, 0.0f };
    }

    // --- Fonctions utilitaires de configuration ---

    void SetAsDirectional(XMFLOAT3 direction, XMFLOAT3 color = { 1.f, 1.f, 1.f }, float intensity = 1.f) {
        m_type = LightType::Directional;
        m_color = color;
        m_intensity = intensity;
        m_dirDirection = direction;
    }

    void SetAsPoint(float range, XMFLOAT3 attenuation = { 1.f, 0.1f, 0.01f }, XMFLOAT3 color = { 1.f, 1.f, 1.f }, float intensity = 1.f) {
        m_type = LightType::Point;
        m_color = color;
        m_intensity = intensity;
        m_pointRange = range;
        m_pointAttenuation = attenuation;
    }

    void SetAsSpot(XMFLOAT3 direction, float range, float innerAngle, float outerAngle, XMFLOAT3 attenuation = { 1.f, 0.1f, 0.01f }, XMFLOAT3 color = { 1.f, 1.f, 1.f }, float intensity = 1.f) {
        m_type = LightType::Spot;
        m_color = color;
        m_intensity = intensity;
        m_spotDirection = direction;
        m_spotRange = range;
        m_spotInnerAngle = innerAngle;
        m_spotOuterAngle = outerAngle;
        m_spotAttenuation = attenuation;
    }

    virtual const ComponentType::Type GetType() override { return ComponentType::Light; }
};