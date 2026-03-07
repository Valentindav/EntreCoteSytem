#pragma once
#include <ECS_Engine.h>
#include "GameManager.h"

START_SCRIPT(BulletScript)

enum SwordState
{
    Idle,
    Flying,
    Returning,
};

float m_speed = 60.0f;
float m_returnSpeed = 140.0f;
float m_maxRange = 80.0f;
float m_pickupRadius = 1.5f;
float m_arcHeight = 8.0f;
int damage = 10;

Entity* shooter = nullptr;
SwordState  m_state = SwordState::Idle;

private:
    float m_distanceTravelled = 0.0f;
    DirectX::XMFLOAT3 m_flyDir = { 0,0,1 };
    DirectX::XMFLOAT3 m_originalFlyDir = { 0,0,1 };
    DirectX::XMFLOAT3 m_returnStart = { 0,0,0 };
    DirectX::XMFLOAT3 m_returnTarget = { 0,0,0 };
    float m_returnDone = 0.0f;

public:
    bool Left = false;

    void SetFlyDirection(DirectX::XMVECTOR dir)
    {
        DirectX::XMStoreFloat3(&m_flyDir, dir);
        Normalize(m_flyDir);
        m_originalFlyDir = m_flyDir;
    }

    DirectX::XMFLOAT3 GetOriginalFlyDirection() const
    {
        return m_originalFlyDir;
    }

    void Launch()
    {
        if (m_state != SwordState::Idle) return;
        m_distanceTravelled = 0.0f;
        m_state = SwordState::Flying;
    }

    bool IsAvailable() const { return m_state == SwordState::Idle; }

    void OnStart()
    {
        MeshComponent* mesh = ECS_ECS->AddComponents<MeshComponent>(owner);
        mesh->LoadMesh("../../res/sushiSword.obj");

        owner->transform.SetWorldScale({ 0.5f, 0.5f, 0.5f });
        MaterialComponent* mat = ECS_ECS->AddComponents<MaterialComponent>(owner);
        owner->GetComponent<MaterialComponent>()->SetColor({ 1.0f, 0.0f, 0.0f, 1.0f });

        RigidBodyComponent* rb = ECS_ECS->AddComponents<RigidBodyComponent>(owner);
        rb->m_useGravity = false;
        rb->m_motionType = MotionType::Kinematic;
        rb->m_trigger = false;

        ColliderComponent* col = ECS_ECS->AddComponents<ColliderComponent>(owner);
        col->SetAsBox({ 0.5f, 0.5f, 0.5f });

        owner->SetType(Entity::TYPE::Bullet);

    }

    void OnUpdate()
    {
        if (!shooter) return;
        switch (m_state)
        {
        case SwordState::Idle:
        {
            XMFLOAT3 shooterPos = shooter->transform.GetWorldPosition();
            if (!Left)
                owner->transform.SetWorldPosition({ shooterPos.x + 0.7f, shooterPos.y + 0.8f, shooterPos.z });
            else
                owner->transform.SetWorldPosition({ shooterPos.x - 0.3f, shooterPos.y + 0.8f, shooterPos.z });
            owner->transform.SetWorldRotation({ 90.0f, 110.0f, 0.0f });
            break;
        }

        case SwordState::Flying:
        {
            float dt = ECS_APP->GetTimer().DeltaTime();

            float step = m_speed * dt;
            owner->transform.LocalTranslate({m_flyDir.x * step,m_flyDir.y * step,m_flyDir.z * step});
            m_distanceTravelled += step;

            owner->transform.WorldRotate({ 0.0f, 10.0f * dt, 0.0f });

            if (m_distanceTravelled >= m_maxRange)
            {
                m_returnStart = owner->transform.GetWorldPosition();
                m_returnTarget = shooter->transform.GetWorldPosition();
                m_returnDone = 0.0f;
                m_state = SwordState::Returning;
            }
            break;
        }

        case SwordState::Returning:
        {
            float dt = ECS_APP->GetTimer().DeltaTime();

            XMFLOAT3 start = m_returnStart;
            XMFLOAT3 target = shooter->transform.GetWorldPosition();

            float totalDist = sqrtf((target.x - start.x) * (target.x - start.x) +(target.y - start.y) * (target.y - start.y) +(target.z - start.z) * (target.z - start.z));

            float tStep;
            if (totalDist > 0.01f) {
                tStep = (m_returnSpeed * dt) / totalDist;
            }
            else tStep = 1.0f;

            m_returnDone += tStep;
            if (m_returnDone > 1.0f) m_returnDone = 1.0f;

            float t = m_returnDone;

            XMFLOAT3 linear = {start.x + t * (target.x - start.x),start.y + t * (target.y - start.y),start.z + t * (target.z - start.z)};

            float arcOffset = m_arcHeight * 4.0f * t * (1.0f - t);

            owner->transform.SetWorldPosition({ linear.x, linear.y + arcOffset, linear.z });

            owner->transform.WorldRotate({ 0.0f, -10.0f * dt, 0.0f });

            if (m_returnDone >= 1.0f)
            {
                float dist = sqrtf((target.x - owner->transform.GetWorldPosition().x) *(target.x - owner->transform.GetWorldPosition().x) +
                (target.y - owner->transform.GetWorldPosition().y) *(target.y - owner->transform.GetWorldPosition().y) +(target.z - owner->transform.GetWorldPosition().z) *(target.z - owner->transform.GetWorldPosition().z));

                if (dist < m_pickupRadius)
                    m_state = SwordState::Idle;
            }
            break;
        }
        }
    }

    void OnCollisionEnter(Entity* other)
    {
        if (!other || other == owner || other == shooter || other->GetType() == Entity::TYPE::Bullet) return;

        if (m_state == SwordState::Flying)
        {
            m_returnStart = owner->transform.GetWorldPosition();
            m_returnTarget = shooter->transform.GetWorldPosition();
            m_returnDone = 0.0f;
            m_state = SwordState::Returning;
        }
    }

    void OnDestroy() {}

    END_SCRIPT(BulletScript)


    START_SCRIPT(ShootScript)
private:
    Entity* m_sword1 = nullptr;
    Entity* m_sword2 = nullptr;
    BulletScript* bs1 = nullptr;
    BulletScript* bs2 = nullptr;

    BulletScript* SpawnSword(bool left)
    {
        Entity* sword = ECS_ECS->CreateEntity(); 

        ScriptComponent* sc = ECS_ECS->AddComponents<ScriptComponent>(sword);
        sc->SetScript<BulletScript>();

        BulletScript* ss = static_cast<BulletScript*>(sc->m_instance);
        ss->shooter = owner;
        ss->damage = 10;

		if (left) ss->Left = true;
        return ss;
    }

public:
    void OnStart()
    {

        bs1 = SpawnSword(false);
        bs2 = SpawnSword(true);
    }

    void OnUpdate()
    {
        if (Inputs::IsMouseDown(Mouse::LEFT))
        {
            BulletScript* target = nullptr;
            if (bs1 && bs1->IsAvailable())       target = bs1;
            else if (bs2 && bs2->IsAvailable())  target = bs2;

            if (target)
            {
                Entity* camEntity = GameManager::Getinstance()->GetCameras()[0];
                CameraComponent* camComp = camEntity->GetComponent<CameraComponent>();

                DirectX::XMFLOAT3 camPos = camEntity->transform.GetWorldPosition();
                DirectX::XMVECTOR startVec = DirectX::XMLoadFloat3(&camPos);

                DirectX::XMFLOAT2 mousePos = Inputs::GetMousePosition();
                float ndcX = (2.0f * mousePos.x) / ECS_ENGINE->GetClientWidth() - 1.0f;
                float ndcY = 1.0f - (2.0f * mousePos.y) / ECS_ENGINE->GetClientHeight();

                DirectX::XMVECTOR ndcVec = DirectX::XMVectorSet(ndcX, ndcY, 0.0f, 1.0f);
                DirectX::XMMATRIX viewProj = camComp->m_renderCamera->GetView() * camComp->m_renderCamera->GetProj();
                DirectX::XMMATRIX invViewProj = DirectX::XMMatrixInverse(nullptr, viewProj);
                DirectX::XMVECTOR targetVec = DirectX::XMVector3TransformCoord(ndcVec, invViewProj);

                DirectX::XMVECTOR dirVec = DirectX::XMVector3Normalize(
                    DirectX::XMVectorSubtract(targetVec, startVec));

                DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
                DirectX::XMMATRIX lookAt = DirectX::XMMatrixLookToLH(startVec, dirVec, up);
                DirectX::XMVECTOR quatVec = DirectX::XMQuaternionRotationMatrix(
                    DirectX::XMMatrixTranspose(lookAt));

                DirectX::XMFLOAT4 rotation;
                DirectX::XMStoreFloat4(&rotation, quatVec);

                target->owner->transform.SetWorldPosition(camPos);
                target->owner->transform.SetLocalRotation(rotation);

                target->SetFlyDirection(dirVec);
                target->Launch();
            }
        }
    }

    void OnDestroy() {}

    END_SCRIPT(ShootScript)