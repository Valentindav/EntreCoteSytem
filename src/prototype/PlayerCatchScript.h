#pragma once
#include <ECS_engine.h>

#include <DirectXMath.h>
#include <iostream>

using namespace DirectX;

START_SCRIPT(PlayerCatchScript)

Entity* m_cameraPlayer = nullptr;
Entity* m_hitboxPlayer = nullptr;

Entity* m_entityHighLighted = nullptr;
XMFLOAT4 m_originalColor = { 1.f, 0.f, 0.f, 1.f };

float m_catchRange = 2.f;

void SetCamera(Entity* _camEntity) { m_cameraPlayer = _camEntity; }
void SetHitbox(Entity* _hitboxEntity) { m_hitboxPlayer = _hitboxEntity; }

void OnUpdate() override
{
    if (!m_cameraPlayer) return;

    XMFLOAT3 currentPos = m_cameraPlayer->transform.GetWorldPosition();
    XMFLOAT3 currentForward = m_cameraPlayer->transform.GetWorldForward();

    RaycastHit hit;
    Entity* currentHitEntity = nullptr;

    if (Physics::Raycast(currentPos, currentForward, m_catchRange, hit, m_hitboxPlayer) && hit.entity)
    {
        TagComponent* tag = hit.entity->GetComponent<TagComponent>();
        if (tag && tag->m_tag == "catchable")
        {
            currentHitEntity = hit.entity;
        }
    }

    if (m_entityHighLighted == currentHitEntity)
    {
        return;
    }

    if (m_entityHighLighted)
    {
        MaterialComponent* oldMat = m_entityHighLighted->GetComponent<MaterialComponent>();
        if (oldMat) oldMat->SetColor(m_originalColor);

        m_entityHighLighted = nullptr;
    }

    if (currentHitEntity)
    {
        m_entityHighLighted = currentHitEntity;

        MaterialComponent* newMat = m_entityHighLighted->GetComponent<MaterialComponent>();
        if (newMat)
        {
            XMFLOAT4 highlightColor = { 1.f, 1.f, 1.f, 1.f };
            newMat->SetColor(highlightColor);
        }
    }
}

END_SCRIPT(PlayerCatchScript)