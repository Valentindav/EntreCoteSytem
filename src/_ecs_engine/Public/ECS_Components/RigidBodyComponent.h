#pragma once
#include "private/Component.h"

#include <DirectXMath.h>

using namespace DirectX;

enum MotionType : int { Kinematic, Dynamic, Static };

struct MotionProperties
{
    XMFLOAT3 linearVelocity = { 0.f, 0.f, 0.f };
    XMFLOAT3 angularVelocity = { 0.f, 0.f, 0.f };
    XMFLOAT3 acceleration = { 0.f, 0.f, 0.f };

    float linearDamping = 0.98f;
    float angularDamping = 0.98f;
};

struct MassProperties
{
    float mass = 1.0f;
    float inverseMass = 1.0f;
};

class RigidBodyComponent : public Component
{
public:

    float m_bounciness = 0.5f;

    int m_motionType = MotionType::Dynamic;

    MotionProperties m_motionProperties;
    MassProperties m_massProperties;

    XMFLOAT3 m_forceAccumulator = { 0.f, 0.f, 0.f };
    bool m_useGravity = true;

    RigidBodyComponent() = default;

    void SetMass(float mass) {
        if (mass <= 0.0f) {
            m_massProperties.mass = 0.0f;
            m_massProperties.inverseMass = 0.0f;
        }
        else {
            m_massProperties.mass = mass;
            m_massProperties.inverseMass = 1.0f / mass;
        }
    }

    void AddForce(XMFLOAT3 force) {
        XMVECTOR vAcc = XMLoadFloat3(&m_forceAccumulator);
        XMVECTOR vForce = XMLoadFloat3(&force);
        XMStoreFloat3(&m_forceAccumulator, XMVectorAdd(vAcc, vForce));
    }

    void ClearForces() {
        m_forceAccumulator = { 0.f, 0.f, 0.f };
    }

    virtual const ComponentType::Type GetType() { return ComponentType::RigidBody; }

    friend class PhysicSystem;
};