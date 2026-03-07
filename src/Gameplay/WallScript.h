#pragma once
#include <ECS_Engine.h>
#include "ShootingScript.h"
#include "LifeScript.h"

START_SCRIPT(WallScript)
private:
    int m_health = 1;
    int m_damage = 20;
    bool m_destroyed = false;
    float m_destroyTimer = 0.0f;

    Entity* m_particleEmitter = nullptr;
    ParticleEmitterComponent* m_emitterComp = nullptr;

public:
    void OnStart()
    {
        owner->SetType(Entity::TYPE::Ennemy);

        RigidBodyComponent* rb = ECS_ECS->AddComponents<RigidBodyComponent>(owner);
        rb->m_motionType = MotionType::Static;
        rb->m_useGravity = false;

        ColliderComponent* col = ECS_ECS->AddComponents<ColliderComponent>(owner);
        col->SetAsBox({ 0.5f, 0.5f, 0.5f });

        m_particleEmitter = ECS_ECS->CreateEntity();
        m_emitterComp = ECS_ECS->AddComponents<ParticleEmitterComponent>(m_particleEmitter);
    }

    void OnUpdate()
    {
        if (m_destroyed)
        {
            m_particleEmitter->transform.SetWorldPosition(owner->transform.GetWorldPosition());

            m_emitterComp->spawnRadius = 0.5f;
            m_emitterComp->gravity = 2.0f;
            m_emitterComp->speed = 3.0f;
            m_emitterComp->spread = 1.5f;
            m_emitterComp->minLife = 0.5f;
            m_emitterComp->maxLife = 1.0f;
            m_emitterComp->sizeStart = 0.2f;
            m_emitterComp->sizeEnd = 0.05f;
            m_emitterComp->colorStartR = 0.8f;
            m_emitterComp->colorStartG = 0.4f;
            m_emitterComp->colorStartB = 0.0f;
            m_emitterComp->colorStartA = 1.0f;
            m_emitterComp->colorEndR = 0.3f;
            m_emitterComp->colorEndG = 0.3f;
            m_emitterComp->colorEndB = 0.3f;
            m_emitterComp->colorEndA = 0.0f;
            m_emitterComp->maxParticles = 50;
            m_emitterComp->emitRate = 100;

            m_destroyTimer += ECS_APP->GetTimer().DeltaTime();
            if (m_destroyTimer >= 0.5f)
            {
                ECS_ECS->DestroyEntity(m_particleEmitter);
                ECS_ECS->DestroyEntity(owner);
            }
        }
    }

    void OnCollisionEnter(Entity* other)
    {
        if (!other || m_destroyed) return;

        if (other->GetType() == Entity::TYPE::Bullet)
        {
            m_health--;

            if (m_health <= 0)
            {
                m_destroyed = true;
                m_destroyTimer = 0.0f;
                ScriptComponent* bulletSc = other->GetComponent<ScriptComponent>();
                if (bulletSc && bulletSc->m_instance)
                {
                    BulletScript* bs = dynamic_cast<BulletScript*>(bulletSc->m_instance);
                    if (bs && bs->shooter)
                    {
                        std::vector<ScriptComponent*> shooterScripts = bs->shooter->GetComponents<ScriptComponent>();
                        for (ScriptComponent* script : shooterScripts)
                        {
                            PointScript* ps = dynamic_cast<PointScript*>(script->m_instance);
                            if (ps)
                            {
                                ps->AddPoint(10);
                                break;
                            }
                        }
                    }
                }

                owner->transform.SetWorldScale({ 0.0f, 0.0f, 0.0f });
            }
        }

        if (other->GetType() == Entity::TYPE::Player)
        {
            std::vector<ScriptComponent*> playerScripts = other->GetComponents<ScriptComponent>();
            for (ScriptComponent* sc : playerScripts)
            {
                LifeScript* ls = dynamic_cast<LifeScript*>(sc->m_instance);
                if (ls)
                {
                    ls->TakeDamage(m_damage);
                    break;
                }
            }

            m_destroyed = true;
            m_destroyTimer = 0.0f;
            owner->transform.SetWorldScale({ 0.0f, 0.0f, 0.0f });
        }
    }

    void OnDestroy()
    {
        if (m_particleEmitter)
        {
            ECS_ECS->DestroyEntity(m_particleEmitter);
        }
    }

    END_SCRIPT(WallScript)