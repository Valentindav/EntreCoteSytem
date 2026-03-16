#pragma once
#include <ECS_engine.h>
#include "Physics.h"

#include <DirectXMath.h>
#include <iostream>

using namespace DirectX;

START_SCRIPT(PlayerMovementScript)

float m_speed = 5.0f;
float m_jumpSpeed = 5.0f;
float m_gravity = -15.0f;
float m_verticalVelocity = 0.0f;

bool isMoving = false;

Entity* m_cameraPlayer = nullptr;
Entity* m_meshPlayer = nullptr;
Entity* m_hitboxPlayer = nullptr;

// --- Variables FPS Camera ---
float m_yaw = 0.0f;
float m_pitch = 0.0f;
float m_eyeHeight = 1.6f;

float m_jumpCooldown = 0.1f;
float m_jumpTimer = 1.f;

// --- SETTERS POUR L'APP
void SetCamera(Entity* _camEntity) { m_cameraPlayer = _camEntity; }
void SetMesh(Entity* _meshEntity) { m_meshPlayer = _meshEntity; }
void SetHitbox(Entity* _meshEntity) { m_hitboxPlayer = _meshEntity; }

void OnStart()
{
    if (!owner) return;

    if (m_cameraPlayer) {
        owner->AddChild(m_cameraPlayer);
        m_cameraPlayer->transform.SetLocalPosition({ 0.0f, m_eyeHeight, 0.0f });
    }
}

void OnUpdate()
{
    if (!owner || !m_cameraPlayer) return;

    float dt = ECS_APP->GetTimer().DeltaTime();
    m_jumpTimer += dt;

    // ==========================================
    // --- 1. ROTATION CAMERA (FPS)
    // ==========================================

    DirectX::XMFLOAT2 mouseDelta = Inputs::GetMouseDelta();

    float dx = XMConvertToRadians(0.15f * mouseDelta.x);
    float dy = XMConvertToRadians(0.15f * mouseDelta.y);

    m_yaw += dx;
    m_pitch -= dy;

    float pitchLimit = XM_PI / 2.0f - 0.1f;
    if (m_pitch > pitchLimit) m_pitch = pitchLimit;
    if (m_pitch < -pitchLimit) m_pitch = -pitchLimit;

    float lookX = cosf(m_pitch) * sinf(m_yaw);
    float lookY = sinf(m_pitch);
    float lookZ = cosf(m_pitch) * cosf(m_yaw);

    XMFLOAT3 camWorldPos = m_cameraPlayer->transform.GetWorldPosition();

    XMFLOAT3 lookTarget = {
        camWorldPos.x + lookX,
        camWorldPos.y + lookY,
        camWorldPos.z + lookZ
    };
    m_cameraPlayer->transform.LookAt(lookTarget);

    // ==========================================
    // --- 2. DEPLACEMENT HORIZONTAL
    // ==========================================
    XMFLOAT3 deltaMove = { 0.0f, 0.0f, 0.0f };

    if (Inputs::IsKeyPressed(Keyboard::Q)) deltaMove = deltaMove + m_cameraPlayer->transform.GetWorldLeft();
    if (Inputs::IsKeyPressed(Keyboard::D)) deltaMove = deltaMove + m_cameraPlayer->transform.GetWorldRight();
    if (Inputs::IsKeyPressed(Keyboard::Z)) deltaMove = deltaMove + m_cameraPlayer->transform.GetWorldForward();
    if (Inputs::IsKeyPressed(Keyboard::S)) deltaMove = deltaMove + m_cameraPlayer->transform.GetWorldBackward();

    deltaMove.y = 0.0f;

    float length = sqrt(deltaMove.x * deltaMove.x + deltaMove.z * deltaMove.z);

    if (length > 0.001f)
    {
        deltaMove.x /= length;
        deltaMove.z /= length;

        float moveDist = m_speed * dt;
        XMFLOAT3 finalMove = { deltaMove.x * moveDist, 0.0f, deltaMove.z * moveDist };
        owner->transform.WorldTranslate(finalMove);

        if (m_meshPlayer)
        {
            XMFLOAT3 currentMeshPos = m_meshPlayer->transform.GetWorldPosition();
            float meshForwardX = sinf(m_yaw);
            float meshForwardZ = cosf(m_yaw);
            m_meshPlayer->transform.LookAt({
                currentMeshPos.x + meshForwardX,
                currentMeshPos.y,
                currentMeshPos.z + meshForwardZ
                });

            AnimatorComponent* anim = m_meshPlayer->GetComponentInChildren<AnimatorComponent>();
            if (!isMoving && anim) { anim->Play("Take1", true, 1.0f); }
        }
        isMoving = true;
    }
    else
    {
        if (isMoving && m_meshPlayer)
        {
            AnimatorComponent* anim = m_meshPlayer->GetComponentInChildren<AnimatorComponent>();
            if (anim) anim->Stop();
        }
        isMoving = false;
    }

    // ==========================================
    // --- 3. GRAVITE & SAUT (RAYCAST)
    // ==========================================
    m_verticalVelocity += m_gravity * dt;
    owner->transform.WorldTranslate({ 0.0f, m_verticalVelocity * dt, 0.0f });

    XMFLOAT3 currentPos = owner->transform.GetWorldPosition();

    XMFLOAT3 rayOrigin = { currentPos.x, currentPos.y + 0.5f, currentPos.z };
    XMFLOAT3 rayDir = { 0.0f, -1.0f, 0.0f };

    RaycastHit hit;
    bool isGrounded = false;

    if (Physics::Raycast(rayOrigin, rayDir, 0.55f, hit, m_hitboxPlayer))
    {
        isGrounded = true;
        owner->transform.SetWorldPosition({ currentPos.x, hit.point.y, currentPos.z });

        if (m_verticalVelocity <= 0.0f)
        {
            m_verticalVelocity = 0.0f;
        }
    }

    if (Inputs::IsKeyDown(Keyboard::SPACE) && isGrounded && m_jumpTimer >= m_jumpCooldown)
    {
        m_verticalVelocity = m_jumpSpeed;
        m_jumpTimer = 0.0f;
    }
}

END_SCRIPT(PlayerMovementScript)