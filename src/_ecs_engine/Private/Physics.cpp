#include "Public/ECS_Engine.h"

using namespace DirectX;
using namespace std;

bool Physics::Raycast(XMFLOAT3 origin, XMFLOAT3 direction, float maxDistance, RaycastHit& outHit, Entity* ignoreEntity)
{
    outHit.hit = false;
    float closestHitDistance = maxDistance;

    auto colliders = ECS_ECS->GetComponents<ColliderComponent>();

    XMVECTOR vOriginWorld = XMLoadFloat3(&origin);
    XMVECTOR vDirWorld = XMVector3Normalize(XMLoadFloat3(&direction));

    auto safeInv = [](float d) { return (abs(d) < 1e-6f) ? ((d < 0.0f) ? -1e6f : 1e6f) : 1.0f / d; };

    for (ColliderComponent* collider : colliders)
    {
        if (!collider || collider->m_isTrigger) continue;

        Entity* entity = collider->GetOwner();
        if (!entity || entity == ignoreEntity) continue;

        if (collider->m_type == ColliderType::Box)
        {
            TransformComponent& transform = entity->transform;

            XMMATRIX worldMat = XMLoadFloat4x4(&transform.GetWorldMatrix());
            XMVECTOR det;
            XMMATRIX invWorldMat = XMMatrixInverse(&det, worldMat);

            XMVECTOR vLocalOrigin = XMVector3TransformCoord(vOriginWorld, invWorldMat);
            XMVECTOR vLocalDir = XMVector3TransformNormal(vDirWorld, invWorldMat);

            XMFLOAT3 localOrigin, localDir;
            XMStoreFloat3(&localOrigin, vLocalOrigin);
            XMStoreFloat3(&localDir, vLocalDir);

            XMFLOAT3 halfSize = collider->m_boxHalfSize;

            float t1 = (-halfSize.x - localOrigin.x) * safeInv(localDir.x);
            float t2 = (halfSize.x - localOrigin.x) * safeInv(localDir.x);
            float t3 = (-halfSize.y - localOrigin.y) * safeInv(localDir.y);
            float t4 = (halfSize.y - localOrigin.y) * safeInv(localDir.y);
            float t5 = (-halfSize.z - localOrigin.z) * safeInv(localDir.z);
            float t6 = (halfSize.z - localOrigin.z) * safeInv(localDir.z);

            float tmin = max(max(min(t1, t2), min(t3, t4)), min(t5, t6));
            float tmax = min(min(max(t1, t2), max(t3, t4)), max(t5, t6));

            if (tmax < 0.0f || tmin > tmax) continue;

            float t = (tmin < 0.0f) ? tmax : tmin;

            XMVECTOR localHitPoint = XMVectorAdd(vLocalOrigin, XMVectorScale(vLocalDir, t));
            XMVECTOR worldHitPoint = XMVector3TransformCoord(localHitPoint, worldMat);

            float dist = XMVectorGetX(XMVector3Length(XMVectorSubtract(worldHitPoint, vOriginWorld)));

            if (dist < closestHitDistance)
            {
                closestHitDistance = dist;
                outHit.hit = true;
                outHit.entity = entity;
                outHit.distance = dist;
                XMStoreFloat3(&outHit.point, worldHitPoint);

                XMFLOAT3 localNormal = { 0,0,0 };
                if (t == t1) localNormal = { -1, 0, 0 };
                else if (t == t2) localNormal = { 1, 0, 0 };
                else if (t == t3) localNormal = { 0, -1, 0 };
                else if (t == t4) localNormal = { 0, 1, 0 };
                else if (t == t5) localNormal = { 0, 0, -1 };
                else if (t == t6) localNormal = { 0, 0, 1 };

                XMVECTOR vLocalNormal = XMLoadFloat3(&localNormal);
                XMVECTOR vWorldNormal = XMVector3TransformNormal(vLocalNormal, worldMat);
                XMStoreFloat3(&outHit.normal, XMVector3Normalize(vWorldNormal));
            }
        }
    }
    return outHit.hit;
}