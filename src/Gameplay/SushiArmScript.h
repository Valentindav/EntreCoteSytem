#pragma once
#include <ECS_Engine.h>

START_SCRIPT(SushiArmScript)

public:
    Entity* bodyEntity = nullptr;    
    bool isLeftArm = false;          

    float bobSpeed = 2.0f;           
    float bobAmount = 0.08f;         
    float phaseOffset = 0.0f;        

private:
    float m_time = 0.0f;

public:
    void OnStart()
    {
        m_time = 0.0f;
    }

    void OnUpdate()
    {
        if (!bodyEntity) return;

        float dt = ECS_APP->GetTimer().DeltaTime();
        m_time += dt;

        DirectX::XMFLOAT3 bodyPos = bodyEntity->transform.GetWorldPosition();

        float verticalOffset = std::sin(m_time * bobSpeed + phaseOffset) * bobAmount;

        float xOffset = 0.f;
        if (isLeftArm) {
            xOffset = 0.2f; 
        }
        else {
            xOffset = 0.3f;
        }
        owner->transform.SetWorldPosition({bodyPos.x-0.25f + xOffset,bodyPos.y + verticalOffset,bodyPos.z});

        //owner->transform.SetWorldRotation({ 180.0f, 110.0f, 0.0f });
        owner->transform.SetWorldScale({ 0.4f, 0.4f, 0.4f });
    }

    void OnDestroy() {}

    END_SCRIPT(SushiArmScript)