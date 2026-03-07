#pragma once
#include "Private/Component.h"

#include <Data/TransformData.h>
#include <Helpers/MathHelper.h>

enum Dirty : uint8_t {
    NONE,
    ALL,


};

class Entity;

class TransformComponent : public Component
{
    Transform m_transform;


    Entity* m_pOwner = nullptr;


    XMFLOAT3 m_WorldPosition = { 0,0,0 };
    XMFLOAT3 m_WorldScale={ 1, 1, 1 };;

    XMFLOAT3 m_WorldForward={ 1, 0, 0 };
    XMFLOAT3 m_WorldRight={ 0, 1, 0 };
    XMFLOAT3 m_WorldUp={ 0, 0, 1 };

    XMFLOAT4 m_WorldRot = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

    XMFLOAT4X4 m_WorldMatrix = MathHelper::Identity4x4();

    Dirty m_dirty = Dirty::ALL;

    bool IsDirty(Dirty flags) const {};

public:
    void SetOwner(Entity* ent) { m_pOwner = ent;}

    void SetLocalScale(XMFLOAT3 const& Scale);
    void SetWorldScale(XMFLOAT3 const& Scale);

    void LocalScale(XMFLOAT3 const& factor);
    void WorldScale(XMFLOAT3 const& factor);

    void SetLocalPosition(XMFLOAT3 const& position);
    void SetWorldPosition(XMFLOAT3 const& position);

    void LocalTranslate(XMFLOAT3 const& offset);
    void WorldTranslate(XMFLOAT3 const& offset);

    void SetLocalRotation(XMFLOAT4 const& rotation);
    void SetWorldRotation(XMFLOAT4 const& rotation);

    void SetLocalRotation(XMFLOAT3 const& angles);
    void SetWorldRotation(XMFLOAT3 const& angles);

    void LookAt(const DirectX::XMFLOAT3& target);
    void LocalLookAt(const DirectX::XMFLOAT3& localTarget);

    void LocalRotate(XMFLOAT3 const& angles);
    void WorldRotate(XMFLOAT3 const& angles);

    XMFLOAT3 GetLocalPosition() const { return m_transform.position; }
    XMFLOAT3 GetWorldPosition() const { return m_WorldPosition; }

    XMFLOAT3 GetLocalScale() const { return m_transform.scale; }
    XMFLOAT3 GetWorldScale() const { return m_WorldScale; }

    XMFLOAT3 GetLocalForward() const { return m_transform.forward; }
    XMFLOAT3 GetWorldForward() const { return m_WorldForward; }

    XMFLOAT3 GetLocalLeft() const { return XMFLOAT3(-m_transform.right.x,-m_transform.right.y,-m_transform.right.z); }
    XMFLOAT3 GetWorldLeft() const { return XMFLOAT3(-m_WorldRight.x, -m_WorldRight.y, -m_WorldRight.z); }

    XMFLOAT3 GetLocalRight() const { return m_transform.right; }
    XMFLOAT3 GetWorldRight() const { return m_WorldRight; }

    XMFLOAT3 GetLocalUp() const { return m_transform.up; }
    XMFLOAT3 GetWorldUp() const { return m_WorldUp; }

    XMFLOAT3 GetLocalBackward() const { return XMFLOAT3(-m_transform.forward.x, -m_transform.forward.y, -m_transform.forward.z); }
    XMFLOAT3 GetWorldBackward() const { return XMFLOAT3(-m_WorldForward.x, -m_WorldForward.y, -m_WorldForward.z); }

    XMFLOAT4 const& GetLocalRotation() const { return m_transform.rotation; }
    XMFLOAT4 const& GetWorldRotation() const { return m_WorldRot; }

    XMFLOAT4X4 const& GetWorldMatrix() const { return m_WorldMatrix; }

    void UpdateMatrix();

    void NewParent();
    void RemoveParent();

    virtual const ComponentType::Type GetType() { return ComponentType::Transform; }

private:

    void UpdateLocalPosition();
    void UpdateWorldPosition();

    void UpdateLocalScale();
    void UpdateWorldScale();

    void UpdateLocalRotation();
    void UpdateWorldRotation();

    void UpdateLocalDirections();
    void UpdateWorldDirections();


};