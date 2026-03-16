#include "PhysicSystem.h"
#include "Public/ECS_Components/RigidBodyComponent.h"
#include "Public/ECS_Components/ColliderComponent.h"
#include "Public/ECS_Components/TransformComponent.h"
#include "Public/ECS.h"
#include "Private/EngineCore.h"
#include <algorithm>

const DirectX::XMVECTOR GRAVITY = DirectX::XMVectorSet(0.0f, -9.81f, 0.0f, 0.0f);

void PhysicSystem::UpdateAABB(ColliderComponent* collider, TransformComponent* transform)
{
    transform->UpdateMatrix();

    XMFLOAT3 worldPos = transform->GetWorldPosition();
    XMFLOAT3 worldScale = transform->GetWorldScale();

    XMVECTOR vPos = XMLoadFloat3(&worldPos);
    XMFLOAT3 offset = { 0.f, 0.f, 0.f };
    XMVECTOR vOffset = XMLoadFloat3(&offset);
    XMVECTOR vCenter = XMVectorAdd(vPos, vOffset);

    XMFLOAT3 center;
    XMStoreFloat3(&center, vCenter);

    if (collider->m_type == ColliderType::Sphere)
    {
        float maxScale = max(worldScale.x, max(worldScale.y, worldScale.z));
        float r = collider->m_radius * maxScale;

        collider->m_aabb.min = { center.x - r, center.y - r, center.z - r };
        collider->m_aabb.max = { center.x + r, center.y + r, center.z + r };
    }
    else if (collider->m_type == ColliderType::Box)
    {
        float extentX = collider->m_boxHalfSize.x * worldScale.x;
        float extentY = collider->m_boxHalfSize.y * worldScale.y;
        float extentZ = collider->m_boxHalfSize.z * worldScale.z;

        collider->m_aabb.min = { center.x - extentX, center.y - extentY, center.z - extentZ };
        collider->m_aabb.max = { center.x + extentX, center.y + extentY, center.z + extentZ };
    }
}

void PhysicSystem::AddComponent(Component* _comp)
{
    if (_comp->GetType() == ComponentType::Collider)
    {
        m_colliders.push_back(static_cast<ColliderComponent*>(_comp));
    }
    if (_comp->GetType() == ComponentType::RigidBody)
    {
        m_rigidBodies.push_back(static_cast<RigidBodyComponent*>(_comp));
    }
}

void PhysicSystem::Update()
{
    m_timerFixedUpdate += ECS_ENGINE->GetTimer().DeltaTime();

    const float fixedDeltaTime = 1.f / 60.f;

    while (m_timerFixedUpdate >= fixedDeltaTime)
    {
        for (RigidBodyComponent* rb : m_rigidBodies)
        {
            if (rb && rb->m_motionType == MotionType::Dynamic)
            {
                UpdatePhysic(rb, fixedDeltaTime);
            }
        }

        for (ColliderComponent* col : m_colliders)
        {
            col->m_entitiesCollided.clear();

            Entity* owner = col->GetOwner();
            if (owner)
            {
                UpdateAABB(col, &owner->transform);
            }
        }

        m_potentialCollisions.clear();
        BroadPhase();
        NarrowPhase();

        m_timerFixedUpdate -= fixedDeltaTime;
    }

    m_colliders.clear();
    m_rigidBodies.clear();
}

void PhysicSystem::UpdatePhysic(RigidBodyComponent* rb, float dt)
{
    int entityID = rb->GetOwnerID();
    Entity* entity = ECS_ENGINE->GetECS()->GetEntity(entityID);
    if (!entity) return;
    TransformComponent* transform = &entity->transform;

    XMFLOAT3 worldPos = transform->GetWorldPosition();
    XMVECTOR vPos = XMLoadFloat3(&worldPos);
    XMVECTOR vVel = XMLoadFloat3(&rb->m_motionProperties.linearVelocity);
    XMVECTOR vForce = XMLoadFloat3(&rb->m_forceAccumulator);

    // Gravity
    if (rb->m_useGravity) {
        XMVECTOR vGravityForce = XMVectorScale(GRAVITY, rb->m_massProperties.mass);
        vForce = XMVectorAdd(vForce, vGravityForce);
    }

    // Intégration de Newton (F = ma => a = F * (1/m))
    XMVECTOR vAccel = XMVectorScale(vForce, rb->m_massProperties.inverseMass);
    XMStoreFloat3(&rb->m_motionProperties.acceleration, vAccel);

    // Vitesse += Acceleration * dt
    vVel = XMVectorAdd(vVel, XMVectorScale(vAccel, dt));

    // Friction
    vVel = XMVectorScale(vVel, rb->m_motionProperties.linearDamping);

    // Deplacement
    vPos = XMVectorAdd(vPos, XMVectorScale(vVel, dt));

    XMFLOAT3 newPos = transform->GetWorldPosition();
    XMStoreFloat3(&newPos, vPos);
    XMStoreFloat3(&rb->m_motionProperties.linearVelocity, vVel);

    transform->SetWorldPosition(newPos);
    transform->UpdateMatrix();

    rb->ClearForces();
}

void PhysicSystem::BroadPhase()
{
    m_grid.clear();

    for (int i = 0; i < m_colliders.size(); i++)
    {
        AABB& aabb = m_colliders[i]->m_aabb;

        int minX = static_cast<int>(floor(aabb.min.x / m_cellSize));
        int maxX = static_cast<int>(floor(aabb.max.x / m_cellSize));
        int minY = static_cast<int>(floor(aabb.min.y / m_cellSize));
        int maxY = static_cast<int>(floor(aabb.max.y / m_cellSize));
        int minZ = static_cast<int>(floor(aabb.min.z / m_cellSize));
        int maxZ = static_cast<int>(floor(aabb.max.z / m_cellSize));

        for (int x = minX; x <= maxX; ++x) {
            for (int y = minY; y <= maxY; ++y) {
                for (int z = minZ; z <= maxZ; ++z) {
                    uint64_t hash = ComputeHash(x, y, z);
                    m_grid[hash].push_back(i);
                }
            }
        }
    }

    for (auto const& [hash, cellIndices] : m_grid)
    {
        if (cellIndices.size() < 2) continue;

        for (size_t i = 0; i < cellIndices.size(); i++)
        {
            for (size_t j = i + 1; j < cellIndices.size(); j++)
            {
                int idxA = cellIndices[i];
                int idxB = cellIndices[j];

                if (m_colliders[idxA]->m_aabb.Intersects(m_colliders[idxB]->m_aabb))
                {
                    int idA = m_colliders[idxA]->mOwnerID;
                    int idB = m_colliders[idxB]->mOwnerID;

                    if (idA < idB)
                        m_potentialCollisions.push_back({ idA, idB });
                    else
                        m_potentialCollisions.push_back({ idB, idA });
                }
            }
        }
    }

    std::sort(m_potentialCollisions.begin(), m_potentialCollisions.end());
    m_potentialCollisions.erase(std::unique(m_potentialCollisions.begin(), m_potentialCollisions.end()), m_potentialCollisions.end());
}

void PhysicSystem::NarrowPhase()
{
    ECS* ecs = ECS_ENGINE->GetECS();

    for (const auto& pair : m_potentialCollisions)
    {
        Entity* eA = ecs->GetEntity(pair.first);
        Entity* eB = ecs->GetEntity(pair.second);

        if (!eA || !eB) continue;

        auto collidersA = eA->GetComponents<ColliderComponent>();
        auto collidersB = eB->GetComponents<ColliderComponent>();

        if (collidersA.empty() || collidersB.empty()) continue;

        ColliderComponent* cA = collidersA[0];
        ColliderComponent* cB = collidersB[0];
        TransformComponent* tA = &eA->transform;
        TransformComponent* tB = &eB->transform;

        CollisionManifold manifold = { eA, eB, {0,0,0}, 0, false };

        if (cA->m_type == ColliderType::Sphere && cB->m_type == ColliderType::Sphere) {
            manifold = CheckSphereSphere(cA, tA, cB, tB);
        }
        else if (cA->m_type == ColliderType::Box && cB->m_type == ColliderType::Box) {
            manifold = CheckBoxBox(cA, tA, cB, tB);
        }
        else if (cA->m_type == ColliderType::Sphere && cB->m_type == ColliderType::Box) {
            manifold = CheckSphereBox(cA, tA, cB, tB);
        }
        else if (cA->m_type == ColliderType::Box && cB->m_type == ColliderType::Sphere) {
            manifold = CheckSphereBox(cB, tB, cA, tA);
            manifold.normal.x *= -1; manifold.normal.y *= -1; manifold.normal.z *= -1;
            std::swap(manifold.entityA, manifold.entityB);
        }

        if (manifold.hasCollision)
        {
            if (std::find(cA->m_entitiesCollided.begin(), cA->m_entitiesCollided.end(), manifold.entityB) == cA->m_entitiesCollided.end()) {
                cA->m_entitiesCollided.push_back(manifold.entityB);
            }
            if (std::find(cB->m_entitiesCollided.begin(), cB->m_entitiesCollided.end(), manifold.entityA) == cB->m_entitiesCollided.end()) {
                cB->m_entitiesCollided.push_back(manifold.entityA);
            }

            if (!cA->m_isTrigger && !cB->m_isTrigger)
            {
                ResolveCollision(manifold);
            }
        }
    }
}

CollisionManifold PhysicSystem::CheckSphereSphere(ColliderComponent* A, TransformComponent* tA, ColliderComponent* B, TransformComponent* tB)
{
    CollisionManifold m;
    m.entityA = tA->GetOwner();
    m.entityB = tB->GetOwner();
    m.hasCollision = false;

    XMFLOAT3 worldPosA = tA->GetWorldPosition();
    XMFLOAT3 worldPosB = tB->GetWorldPosition();

    XMVECTOR posA = XMLoadFloat3(&worldPosA);
    XMVECTOR posB = XMLoadFloat3(&worldPosB);

    XMVECTOR vDelta = XMVectorSubtract(posB, posA);
    float distSq = XMVectorGetX(XMVector3LengthSq(vDelta));
    float dist = sqrt(distSq);

    float radiusSum = A->m_radius + B->m_radius;

    if (dist < radiusSum)
    {
        m.hasCollision = true;
        m.depth = radiusSum - dist;

        if (dist > 0.0001f)
            XMStoreFloat3(&m.normal, XMVectorScale(vDelta, 1.0f / dist));
        else
            m.normal = { 0, 1, 0 };
    }

    return m;
}

CollisionManifold PhysicSystem::CheckBoxBox(ColliderComponent* A, TransformComponent* tA, ColliderComponent* B, TransformComponent* tB)
{
    CollisionManifold m;
    m.entityA = tA->GetOwner();
    m.entityB = tB->GetOwner();
    m.hasCollision = false;

    float minPenetration = FLT_MAX;
    XMVECTOR bestAxis = XMVectorSet(0, 0, 0, 0);

    XMFLOAT3 worldPosA = tA->GetWorldPosition();
    XMFLOAT3 worldPosB = tB->GetWorldPosition();
    XMVECTOR pA = XMLoadFloat3(&worldPosA);
    XMVECTOR pB = XMLoadFloat3(&worldPosB);

    XMVECTOR vCenterDiff = XMVectorSubtract(pB, pA);

    XMMATRIX matA = XMLoadFloat4x4(&tA->GetWorldMatrix());
    XMMATRIX matB = XMLoadFloat4x4(&tB->GetWorldMatrix());

    XMVECTOR axisA[3] = { XMVector3Normalize(matA.r[0]), XMVector3Normalize(matA.r[1]), XMVector3Normalize(matA.r[2]) };
    XMVECTOR axisB[3] = { XMVector3Normalize(matB.r[0]), XMVector3Normalize(matB.r[1]), XMVector3Normalize(matB.r[2]) };

    XMFLOAT3 scaleA = tA->GetWorldScale();
    XMVECTOR eA = XMVectorSet(
        abs(A->m_boxHalfSize.x * scaleA.x),
        abs(A->m_boxHalfSize.y * scaleA.y),
        abs(A->m_boxHalfSize.z * scaleA.z), 0.0f);

    XMFLOAT3 scaleB = tB->GetWorldScale();
    XMVECTOR eB = XMVectorSet(
        abs(B->m_boxHalfSize.x * scaleB.x),
        abs(B->m_boxHalfSize.y * scaleB.y),
        abs(B->m_boxHalfSize.z * scaleB.z), 0.0f);

    auto TestAxis = [&](XMVECTOR axis) -> bool
        {
            if (XMVectorGetX(XMVector3LengthSq(axis)) < 0.0001f) return true;

            axis = XMVector3Normalize(axis);

            float rA = abs(XMVectorGetX(XMVector3Dot(axisA[0], axis))) * XMVectorGetX(eA) +
                abs(XMVectorGetX(XMVector3Dot(axisA[1], axis))) * XMVectorGetY(eA) +
                abs(XMVectorGetX(XMVector3Dot(axisA[2], axis))) * XMVectorGetZ(eA);

            float rB = abs(XMVectorGetX(XMVector3Dot(axisB[0], axis))) * XMVectorGetX(eB) +
                abs(XMVectorGetX(XMVector3Dot(axisB[1], axis))) * XMVectorGetY(eB) +
                abs(XMVectorGetX(XMVector3Dot(axisB[2], axis))) * XMVectorGetZ(eB);

            float distProj = abs(XMVectorGetX(XMVector3Dot(vCenterDiff, axis)));

            float penetration = (rA + rB) - distProj;

            if (penetration < 0) return false;

            if (penetration < minPenetration)
            {
                minPenetration = penetration;
                bestAxis = axis;
            }
            return true;
        };

    for (int i = 0; i < 3; i++) if (!TestAxis(axisA[i])) return m;

    for (int i = 0; i < 3; i++) if (!TestAxis(axisB[i])) return m;

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            XMVECTOR crossAxis = XMVector3Cross(axisA[i], axisB[j]);
            if (!TestAxis(crossAxis)) return m;
        }
    }

    m.hasCollision = true;
    m.depth = minPenetration;

    if (XMVectorGetX(XMVector3Dot(bestAxis, vCenterDiff)) < 0)
    {
        bestAxis = XMVectorScale(bestAxis, -1.0f);
    }

    XMStoreFloat3(&m.normal, bestAxis);

    return m;
}

CollisionManifold PhysicSystem::CheckSphereBox(ColliderComponent* A, TransformComponent* tA, ColliderComponent* B, TransformComponent* tB)
{
    return CollisionManifold();
}

void PhysicSystem::ResolveCollision(const CollisionManifold& m)
{
    if (!m.entityA || !m.entityB) return;

    RigidBodyComponent* rbA = m.entityA->GetComponentInHierarchy<RigidBodyComponent>();
    RigidBodyComponent* rbB = m.entityB->GetComponentInHierarchy<RigidBodyComponent>();

    if (!rbA && !rbB) return;

    TransformComponent* transformA = rbA ? &rbA->GetOwner()->transform : &m.entityA->transform;
    TransformComponent* transformB = rbB ? &rbB->GetOwner()->transform : &m.entityB->transform;

    bool dynA = (rbA && rbA->m_motionType == MotionType::Dynamic);
    bool dynB = (rbB && rbB->m_motionType == MotionType::Dynamic);
    bool kinA = (rbA && rbA->m_motionType == MotionType::Kinematic);
    bool kinB = (rbB && rbB->m_motionType == MotionType::Kinematic);
    bool statA = (!rbA || rbA->m_motionType == MotionType::Static);
    bool statB = (!rbB || rbB->m_motionType == MotionType::Static);

    float moveA = 0.0f;
    float moveB = 0.0f;

    if (dynA && dynB) {
        moveA = rbA->m_massProperties.inverseMass;
        moveB = rbB->m_massProperties.inverseMass;
    }
    else if (dynA) {
        moveA = 1.0f; moveB = 0.0f;
    }
    else if (dynB) {
        moveA = 0.0f; moveB = 1.0f;
    }
    else if (kinA && statB) {
        moveA = 1.0f; moveB = 0.0f;
    }
    else if (statA && kinB) {
        moveA = 0.0f; moveB = 1.0f;
    }
    else if (kinA && kinB) {
        moveA = 0.5f; moveB = 0.5f;
    }
    else {
        return;
    }

    float totalMove = moveA + moveB;
    if (totalMove <= 0.0f) return;

    float percent = 0.8f;
    float slop = 0.01f;
    XMVECTOR vNormal = XMLoadFloat3(&m.normal);

    float correctionMagnitude = (max(m.depth - slop, 0.0f) / totalMove) * percent;
    XMVECTOR vCorrection = XMVectorScale(vNormal, correctionMagnitude);

    if (moveA > 0.0f)
    {
        XMFLOAT3 delta;
        XMStoreFloat3(&delta, XMVectorScale(vCorrection, -moveA));
        transformA->WorldTranslate(delta);
    }

    if (moveB > 0.0f)
    {
        XMFLOAT3 delta;
        XMStoreFloat3(&delta, XMVectorScale(vCorrection, moveB));
        transformB->WorldTranslate(delta);
    }

    if (!dynA && !dynB) return;

    float invMassA = dynA ? rbA->m_massProperties.inverseMass : 0.0f;
    float invMassB = dynB ? rbB->m_massProperties.inverseMass : 0.0f;
    float totalInvMass = invMassA + invMassB;

    if (totalInvMass <= 0.0f) return;

    XMVECTOR velA = (dynA) ? XMLoadFloat3(&rbA->m_motionProperties.linearVelocity) : XMVectorZero();
    XMVECTOR velB = (dynB) ? XMLoadFloat3(&rbB->m_motionProperties.linearVelocity) : XMVectorZero();

    XMVECTOR relativeVel = XMVectorSubtract(velB, velA);
    float velAlongNormal = XMVectorGetX(XMVector3Dot(relativeVel, vNormal));

    if (velAlongNormal > 0) return;

    float bouncinessA = (rbA) ? rbA->m_bounciness : 0.5f;
    float bouncinessB = (rbB) ? rbB->m_bounciness : 0.5f;
    float e = min(bouncinessA, bouncinessB);

    float j = -(1 + e) * velAlongNormal;
    j /= totalInvMass;

    XMVECTOR impulse = XMVectorScale(vNormal, j);

    if (dynA) {
        XMVECTOR newVelA = XMVectorSubtract(velA, XMVectorScale(impulse, invMassA));
        XMStoreFloat3(&rbA->m_motionProperties.linearVelocity, newVelA);
    }

    if (dynB) {
        XMVECTOR newVelB = XMVectorAdd(velB, XMVectorScale(impulse, invMassB));
        XMStoreFloat3(&rbB->m_motionProperties.linearVelocity, newVelB);
    }
}