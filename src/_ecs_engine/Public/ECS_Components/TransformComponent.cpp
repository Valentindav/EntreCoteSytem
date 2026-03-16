#include "TransformComponent.h"
#include "Public/Entity.h"

using namespace DirectX;

void TransformComponent::SetLocalScale(XMFLOAT3 const& Scale)
{
    m_transform.scale = Scale;
    UpdateWorldScale();
    m_dirty = Dirty::ALL;
}

void TransformComponent::SetWorldScale(XMFLOAT3 const& Scale)
{
    m_WorldScale = Scale;
    UpdateLocalScale();
    m_dirty = Dirty::ALL;
}

void TransformComponent::LocalScale(XMFLOAT3 const& factor)
{
    XMVECTOR vector = XMLoadFloat3(&m_transform.scale);
    XMVECTOR vector2 = XMLoadFloat3(&factor);
    vector = XMVectorMultiply(vector, vector2);
    XMStoreFloat3(&m_transform.scale, vector);
    UpdateWorldScale();
    m_dirty = Dirty::ALL;
}

void TransformComponent::WorldScale(XMFLOAT3 const& factor)
{
    XMVECTOR vector = XMLoadFloat3(&m_WorldScale);
    XMVECTOR vector2 = XMLoadFloat3(&factor);
    vector = XMVectorMultiply(vector, vector2);
    XMStoreFloat3(&m_WorldScale, vector);
    UpdateLocalScale();
    m_dirty = Dirty::ALL;
}

void TransformComponent::SetLocalPosition(XMFLOAT3 const& position)
{
    m_transform.position = position;
    UpdateWorldPosition();
    m_dirty = Dirty::ALL;
}

void TransformComponent::SetWorldPosition(XMFLOAT3 const& position)
{
    m_WorldPosition = position;
    UpdateLocalPosition();
    m_dirty = Dirty::ALL;
}

void TransformComponent::LocalTranslate(XMFLOAT3 const& offset)
{
    XMVECTOR vector = XMLoadFloat3(&m_transform.position);
    XMVECTOR vector2 = XMLoadFloat3(&offset);
    vector = XMVectorAdd(vector, vector2);
    XMStoreFloat3(&m_transform.position, vector);
    UpdateWorldPosition();
    m_dirty = Dirty::ALL;
}

void TransformComponent::WorldTranslate(XMFLOAT3 const& offset)
{
    XMVECTOR vector = XMLoadFloat3(&m_WorldPosition);
    XMVECTOR vector2 = XMLoadFloat3(&offset);
    vector = XMVectorAdd(vector, vector2);
    XMStoreFloat3(&m_WorldPosition, vector);
    UpdateLocalPosition();
    m_dirty = Dirty::ALL;
}

void TransformComponent::SetLocalRotation(XMFLOAT4 const& rotation)
{
    m_transform.rotation = rotation;
    UpdateWorldRotation();
    UpdateLocalDirections();
    m_dirty = Dirty::ALL;
}

void TransformComponent::SetWorldRotation(XMFLOAT4 const& rotation)
{
    m_WorldRot = rotation;
    UpdateLocalRotation();
    UpdateWorldDirections();
    m_dirty = Dirty::ALL;
}

void TransformComponent::SetLocalRotation(XMFLOAT3 const& angles)
{
    XMVECTOR quat = XMQuaternionRotationRollPitchYaw(
        angles.x,  // pitch
        angles.y,  // yaw
        angles.z   // roll
    );

    XMStoreFloat4(&m_transform.rotation, quat);
    UpdateWorldRotation();
    UpdateLocalDirections();
    m_dirty = Dirty::ALL;
}

void TransformComponent::SetWorldRotation(XMFLOAT3 const& angles)
{
    XMVECTOR quat = XMQuaternionRotationRollPitchYaw(
        angles.x,  // pitch
        angles.y,  // yaw
        angles.z   // roll
    );

    XMStoreFloat4(&m_WorldRot, quat);

    UpdateLocalRotation();
    UpdateWorldDirections();
    m_dirty = Dirty::ALL;
}

void TransformComponent::LookAt(const DirectX::XMFLOAT3& target)
{
    DirectX::XMVECTOR P = DirectX::XMLoadFloat3(&m_WorldPosition);
    DirectX::XMVECTOR T = DirectX::XMLoadFloat3(&target);
    DirectX::XMVECTOR globalUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(P, T, globalUp);
    DirectX::XMVECTOR det = DirectX::XMMatrixDeterminant(viewMatrix);
    DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixInverse(&det, viewMatrix);

    DirectX::XMVECTOR scale;
    DirectX::XMVECTOR rotQuat;
    DirectX::XMVECTOR translation;
    DirectX::XMMatrixDecompose(&scale, &rotQuat, &translation, worldMatrix);

    DirectX::XMFLOAT4 newRot;
    DirectX::XMStoreFloat4(&newRot, rotQuat);

    SetWorldRotation(newRot);

    UpdateMatrix();
}

void TransformComponent::LocalLookAt(const DirectX::XMFLOAT3 & localTarget)
{
    DirectX::XMVECTOR P = DirectX::XMLoadFloat3(&m_transform.position);
    DirectX::XMVECTOR T = DirectX::XMLoadFloat3(&localTarget);

    DirectX::XMVECTOR localUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    if (DirectX::XMVector3Equal(P, T))
        return;

    DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(P, T, localUp);
    DirectX::XMVECTOR det = DirectX::XMMatrixDeterminant(viewMatrix);
    DirectX::XMMATRIX localMatrix = DirectX::XMMatrixInverse(&det, viewMatrix);

    DirectX::XMVECTOR scale;
    DirectX::XMVECTOR rotQuat;
    DirectX::XMVECTOR translation;
    DirectX::XMMatrixDecompose(&scale, &rotQuat, &translation, localMatrix);

    DirectX::XMFLOAT4 newRot;
    DirectX::XMStoreFloat4(&newRot, rotQuat);

    SetLocalRotation(newRot);

    UpdateMatrix();
}

void TransformComponent::LocalRotate(XMFLOAT3 const& angles)
{
    // Axes locaux
    XMVECTOR quatX = XMQuaternionRotationAxis(XMLoadFloat3(&m_transform.right), angles.x);
    XMVECTOR quatY = XMQuaternionRotationAxis(XMLoadFloat3(&m_transform.up), angles.y);
    XMVECTOR quatZ = XMQuaternionRotationAxis(XMLoadFloat3(&m_transform.forward), angles.z);

    XMVECTOR deltaQuat = XMQuaternionMultiply(quatZ, XMQuaternionMultiply(quatY, quatX));
    XMVECTOR currentRot = XMLoadFloat4(&m_transform.rotation);

    XMVECTOR newRot = XMQuaternionMultiply(deltaQuat, currentRot);
    newRot = XMQuaternionNormalize(newRot);

    XMStoreFloat4(&m_transform.rotation, newRot);

    UpdateWorldRotation();
    UpdateLocalDirections();
    m_dirty = Dirty::ALL;
}

void TransformComponent::WorldRotate(XMFLOAT3 const& angles)
{
    XMVECTOR rotation = XMQuaternionRotationRollPitchYaw(angles.x, angles.y, angles.z);
    XMVECTOR currentRot = XMLoadFloat4(&m_WorldRot);

    currentRot = XMQuaternionMultiply(rotation, currentRot);
    currentRot = XMQuaternionNormalize(currentRot);

    XMStoreFloat4(&m_WorldRot, currentRot);
    UpdateLocalRotation();
    UpdateWorldDirections();
    m_dirty = Dirty::ALL;
}

void TransformComponent::UpdateMatrix()
{
    XMVECTOR s = XMLoadFloat3(&m_WorldScale);
    XMVECTOR r = XMLoadFloat4(&m_WorldRot);
    XMVECTOR t = XMLoadFloat3(&m_WorldPosition);

    XMMATRIX S = XMMatrixScalingFromVector(s);
    XMMATRIX R = XMMatrixRotationQuaternion(r);
    XMMATRIX T = XMMatrixTranslationFromVector(t);

    XMMATRIX world = S * R * T;

    XMStoreFloat4x4(&m_WorldMatrix, world);

    for (Entity* const pChild : m_pOwner->GetChildren())
    {
        if (pChild) pChild->transform.UpdateMatrix();
    }
}

void TransformComponent::UpdateWorldPosition()
{
    if (m_pOwner && m_pOwner->HasParent())
    {
        TransformComponent const& parentTransform = m_pOwner->GetParent()->transform;

        XMVECTOR localPos = XMLoadFloat3(&m_transform.position);

        XMFLOAT3 ParentWorldScale = parentTransform.GetWorldScale();
        XMVECTOR parentScale = XMLoadFloat3(&ParentWorldScale);
        localPos = XMVectorMultiply(localPos, parentScale);

        XMFLOAT4 ParentQuat = parentTransform.GetWorldRotation();
        XMVECTOR parentRot = XMLoadFloat4(&ParentQuat);
        localPos = XMVector3Rotate(localPos, parentRot);

        XMFLOAT3 parentWorldPos = parentTransform.GetWorldPosition();
        XMVECTOR parentPos = XMLoadFloat3(&parentWorldPos);
        localPos = XMVectorAdd(localPos, parentPos);

        XMStoreFloat3(&m_WorldPosition, localPos);
    }
    else
    {
        m_WorldPosition = m_transform.position;
    }

    for (Entity* const pChild : m_pOwner->GetChildren())
    {
        if (pChild == nullptr) break;
        pChild->transform.UpdateWorldPosition();
        pChild->transform.UpdateMatrix();
    }
}

void TransformComponent::UpdateLocalPosition()
{
    if (m_pOwner && m_pOwner->HasParent())
    {
        TransformComponent const& parentTransform = m_pOwner->GetParent()->transform;

        XMVECTOR worldPos = XMLoadFloat3(&m_WorldPosition);

        XMFLOAT3 ParentWorldPos = parentTransform.GetWorldPosition();
        XMVECTOR parentPos = XMLoadFloat3(&ParentWorldPos);
        worldPos = XMVectorSubtract(worldPos, parentPos);

        XMFLOAT4 ParentQuat = parentTransform.GetWorldRotation();
        XMVECTOR parentRot = XMLoadFloat4(&ParentQuat);
        XMVECTOR parentRotInv = XMQuaternionInverse(parentRot);
        worldPos = XMVector3Rotate(worldPos, parentRotInv);

        XMFLOAT3 ParentWorldScale = parentTransform.GetWorldScale();
        XMVECTOR parentScale = XMLoadFloat3(&ParentWorldScale);
        worldPos = XMVectorDivide(worldPos, parentScale);

        XMStoreFloat3(&m_transform.position, worldPos);
    }
    else
    {
        m_transform.position = m_WorldPosition;
    }

    for (Entity* const pChild : m_pOwner->GetChildren())
    {
        if (pChild == nullptr) break;
        pChild->transform.UpdateWorldPosition();
    }
}

void TransformComponent::UpdateLocalScale()
{
    m_transform.scale = m_WorldScale;
    if (m_pOwner && m_pOwner->HasParent())
    {
        XMFLOAT3 ParrentWorldscale = m_pOwner->GetParent()->transform.GetWorldScale();

        XMVECTOR currentScale = XMLoadFloat3(&m_transform.scale);
        XMVECTOR ParrentWorldScaleVec = XMLoadFloat3(&ParrentWorldscale);
        currentScale = XMVectorDivide(currentScale, ParrentWorldScaleVec);
        XMStoreFloat3(&m_transform.scale, currentScale);
    }

    for (Entity* const pChild : m_pOwner->GetChildren())
    {
        if (pChild == nullptr) break;
        pChild->transform.UpdateWorldScale();
    }
}

void TransformComponent::UpdateWorldScale()
{
    m_WorldScale = m_transform.scale;
    if (m_pOwner && m_pOwner->HasParent())
    {
        XMFLOAT3 ParrentWorldscale = m_pOwner->GetParent()->transform.GetWorldScale();
        XMVECTOR currentWorldScale = XMLoadFloat3(&m_WorldScale);
        XMVECTOR ParrentWorldScaleVec = XMLoadFloat3(&ParrentWorldscale);

        currentWorldScale = XMVectorMultiply(currentWorldScale, ParrentWorldScaleVec);
        XMStoreFloat3(&m_WorldScale, currentWorldScale);
    }

    for (Entity* const pChild : m_pOwner->GetChildren())
    {
        if (pChild == nullptr) break;
        pChild->transform.UpdateWorldScale();
    }

    UpdateMatrix();
}

void TransformComponent::UpdateLocalRotation()
{
    m_transform.rotation = m_WorldRot;
    if (m_pOwner && m_pOwner->HasParent())
    {
        XMVECTOR ParentrotationInverse = XMLoadFloat4(&m_pOwner->GetParent()->transform.GetWorldRotation());
        ParentrotationInverse = XMQuaternionInverse(ParentrotationInverse);

        XMVECTOR currentRot = XMLoadFloat4(&m_transform.rotation);
        currentRot = XMQuaternionMultiply(currentRot, ParentrotationInverse);
        XMStoreFloat4(&m_transform.rotation, currentRot);
    }
    UpdateLocalDirections();

    for (Entity* const pChild : m_pOwner->GetChildren())
    {
        if (pChild == nullptr) break;
        pChild->transform.UpdateWorldRotation();
    }
}

void TransformComponent::UpdateWorldRotation()
{
    m_WorldRot = m_transform.rotation;
    if (m_pOwner && m_pOwner->HasParent())
    {
        XMVECTOR ParentWorldRot = XMLoadFloat4(&m_pOwner->GetParent()->transform.GetWorldRotation());
        XMVECTOR currentRot = XMLoadFloat4(&m_WorldRot);
        currentRot = XMQuaternionMultiply(currentRot, ParentWorldRot);
        XMStoreFloat4(&m_WorldRot, currentRot);
    }
    UpdateWorldDirections();

    for (Entity* const pChild : m_pOwner->GetChildren())
    {
        if (pChild == nullptr) break;
        pChild->transform.UpdateWorldRotation();
    }

    UpdateMatrix();
    UpdateWorldPosition();
}

void TransformComponent::UpdateLocalDirections()
{
    XMMATRIX localRotationMatrix = XMMatrixRotationQuaternion(XMLoadFloat4(&m_transform.rotation));

    XMStoreFloat3(&m_transform.right, localRotationMatrix.r[0]);
    XMStoreFloat3(&m_transform.up, localRotationMatrix.r[1]);
    XMStoreFloat3(&m_transform.forward, localRotationMatrix.r[2]);
}

void TransformComponent::UpdateWorldDirections()
{
    XMMATRIX worldRotationMatrix = XMMatrixRotationQuaternion(XMLoadFloat4(&m_WorldRot));

    XMStoreFloat3(&m_WorldRight, worldRotationMatrix.r[0]);
    XMStoreFloat3(&m_WorldUp, worldRotationMatrix.r[1]);
    XMStoreFloat3(&m_WorldForward, worldRotationMatrix.r[2]);
}

void TransformComponent::NewParent()
{
    UpdateWorldScale();
    UpdateWorldRotation();
    UpdateWorldPosition();
    UpdateMatrix();
}

void TransformComponent::RemoveParent()
{
    m_transform.position = m_WorldPosition;
    m_transform.scale = m_WorldScale;
    m_transform.rotation = m_WorldRot;
    m_transform.right = m_WorldRight;
    m_transform.up = m_WorldUp;
    m_transform.forward = m_WorldForward;
}