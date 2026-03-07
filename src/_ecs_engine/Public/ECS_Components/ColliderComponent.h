#pragma once
#include "Private/Component.h"

#include <DirectXMath.h>
#include <algorithm>

using namespace DirectX;

enum class ColliderType {
    Sphere,
    Box,
    OBB
};

struct AABB {
    XMFLOAT3 min = { -0.5f, -0.5f, -0.5f };
    XMFLOAT3 max = { 0.5f, 0.5f, 0.5f };

    bool Intersects(const AABB& other) const {
        return (min.x <= other.max.x && max.x >= other.min.x) &&
            (min.y <= other.max.y && max.y >= other.min.y) &&
            (min.z <= other.max.z && max.z >= other.min.z);
    }
};

class ColliderComponent : public Component
{
public:
    ColliderType m_type;

    XMFLOAT3 m_offset = { 0.f, 0.f, 0.f };

    AABB m_aabb;

    union {
        // Données pour une Sphère
        struct {
            float m_radius;
        };

        // Données pour une Box
        struct {
            XMFLOAT3 m_boxHalfSize;
        };

        // Données pour une Capsule
        struct {
            float m_capsuleRadius;
            float m_capsuleHeight;
        };
    };

    ColliderComponent() : m_type(ColliderType::Box) {
        m_boxHalfSize = { 0.5f, 0.5f, 0.5f };
    }

    void SetAsSphere(float radius) {
        m_type = ColliderType::Sphere;
        m_radius = radius;
    }

    void SetAsBox(XMFLOAT3 halfSize) {
        m_type = ColliderType::Box;
        m_boxHalfSize = halfSize;
    }

    virtual const ComponentType::Type GetType() { return ComponentType::Collider; }
};