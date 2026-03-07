#pragma once
#include "Rail.h"
#include <deque>
#include <cmath>
#include <ECS_Engine.h>


START_SCRIPT(RailFollower)
public:
Rail* spline = nullptr;
float  speed = 6.0f;
float  distance = 0.0f;
float  lateralAmplitude = 1.8f;
float  lateralSpeed = 7.0f;

private:
float m_lateralOffset = 0.f;

void OnStart()
{
    m_lateralOffset = 0.f;
    distance = 0.f;
}

void OnUpdate()
{
    if (!spline) return;

    float dt = ECS_APP->GetTimer().DeltaTime();

    distance += speed * dt;

    DirectX::XMFLOAT3 pos = spline->GetPositionAtDistance(distance);
    DirectX::XMFLOAT3 tangent = spline->GetTangentAtDistance(distance);

    float tLen = std::sqrt(tangent.x * tangent.x + tangent.y * tangent.y + tangent.z * tangent.z);
    if (tLen > 1e-4f) { tangent.x /= tLen; tangent.y /= tLen; tangent.z /= tLen; }

    float targetOffset = 0.f;


    m_lateralOffset += (targetOffset - m_lateralOffset) * lateralSpeed * dt;

    DirectX::XMFLOAT3 up = { 0.f, 1.f, 0.f };
    DirectX::XMFLOAT3 right = {
        up.y * tangent.z - up.z * tangent.y,
        up.z * tangent.x - up.x * tangent.z,
        up.x * tangent.y - up.y * tangent.x
    };
    float rLen = std::sqrt(right.x * right.x + right.y * right.y + right.z * right.z);
    if (rLen > 1e-4f) { right.x /= rLen; right.y /= rLen; right.z /= rLen; }

    owner->transform.SetWorldPosition({pos.x + right.x * m_lateralOffset,pos.y,pos.z + right.z * m_lateralOffset});

    float yaw = std::atan2(tangent.x, tangent.z);
    float pitch = -std::asin(std::max<float>(-1.f, std::min<float>(1.f, tangent.y)));
	//owner->transform.SetWorldRotation({ pitch, yaw, 0.f }); -> to comment or not if we want the player to rotate with the rail or not

    spline->UpdateTiles(distance);
}

END_SCRIPT(RailFollower)

START_SCRIPT(EnemyScript)
private:
    float  enemySpeed = 20.f;
    float  Lifetime = 0.0f;
    float m_maxLifeTime = 10.0f;
    int CollideDamage = 10.0f;

    RigidBodyComponent* rb1;
    float DestroyCD = 2.0f;
    bool ToDestroy = false;

    Entity* emitter;
    ParticleEmitterComponent* emitterComp;

    XMFLOAT3 dir;
    XMFLOAT4 rotation;

public:
    bool isDead = false;

    void OnStart()
    {
        owner->transform.SetWorldScale({ 0.6f, 0.6f, 0.6f });
        MeshComponent* mesh3 = ECS_ECS->AddComponents<MeshComponent>(owner);
        mesh3->LoadMesh("../../res/seagull.m3d");

        AnimatorComponent* animator3 = ECS_ECS->AddComponents<AnimatorComponent>(owner);
        animator3->LoadSkeleton("../../res/seagull.m3d");
        animator3->Play("ArmatureAction.001", true, 1.0f);

        MaterialComponent* mat3 = ECS_ECS->AddComponents<MaterialComponent>(owner);
        mat3->LoadTexture("../../res/SeagullTexture.png");

        MaterialComponent* mat = ECS_ECS->AddComponents<MaterialComponent>(owner);
        mat->SetColor({ 1.0f, 0.12f, 0.0f, 1.0f });

        rb1 = ECS_ECS->AddComponents<RigidBodyComponent>(owner);
        rb1->m_motionType = MotionType::Dynamic;
        rb1->m_useGravity = false;

        ColliderComponent* collider1 = ECS_ECS->AddComponents<ColliderComponent>(owner);
        collider1->SetAsBox({ 2.0f, 2.0f, 2.0f });

        ScriptComponent* sc = ECS_ECS->AddComponents<ScriptComponent>(owner);
        sc->SetScript<LifeScript>();

        LifeScript* bs = dynamic_cast<LifeScript*>(sc->m_instance);
        bs->SetLife(100000.0f);

        owner->SetType(Entity::TYPE::Ennemy);

        emitter = ECS_ECS->CreateEntity();
        emitterComp = ECS_ECS->AddComponents<ParticleEmitterComponent>(emitter);
    }

    void Update(DirectX::XMFLOAT3 playerPos)
    {
        if (isDead) return;

        float dt = ECS_APP->GetTimer().DeltaTime();

        if (ToDestroy) {
            DestroyCD -= dt;

            emitter->transform.SetWorldPosition(owner->transform.GetWorldPosition());

            emitterComp->spawnRadius = 0.0001;
            emitterComp->gravity = 0.5f;
            emitterComp->speed = 0.5f;
            emitterComp->spread = 1.0001f;
            emitterComp->minLife = 0.8f;
            emitterComp->maxLife = 0.9f;
            emitterComp->sizeStart = 0.08f;
            emitterComp->sizeEnd = 0.02f;
            emitterComp->colorStartR = 1.0f;  emitterComp->colorStartG = 0.f;  emitterComp->colorStartB = 0.0f;  emitterComp->colorStartA = 1.0f;
            emitterComp->colorEndR = 1.0f;    emitterComp->colorEndG = 0.0f;    emitterComp->colorEndB = 0.5f;    emitterComp->colorEndA = 0.0f;
            emitterComp->maxParticles = 100000;
            emitterComp->emitRate = 100000;

            rb1->AddForce({ dir.x * 100, 20, dir.z * 100 });
            rb1->m_useGravity = true;
            owner->transform.WorldRotate({ rotation.x, rotation.y, rotation.z });

            if (DestroyCD <= 0.0f) {
                ECS_ECS->DestroyEntity(owner);
                ECS_ECS->DestroyEntity(emitter);
            }
            return;
        }

        Lifetime += dt;
        if (Lifetime >= m_maxLifeTime)
        {
            isDead = true;
            ECS_ECS->DestroyEntity(owner);
            return;
        }

        DirectX::XMFLOAT3 ePos = owner->transform.GetWorldPosition();
        float dx = playerPos.x - ePos.x;
        float dy = playerPos.y - ePos.y;
        float dz = playerPos.z - ePos.z;
        float dist = std::sqrt(dx * dx + dy * dy + dz * dz);

        if (dist > 1e-3f) { dx /= dist; dy /= dist; dz /= dist; }
        owner->transform.WorldTranslate({ dx * enemySpeed * dt, dy * enemySpeed * dt, dz * enemySpeed * dt });
    }

    void OnCollisionEnter(Entity* other)
    {
        if (isDead || !other || other->HasToBeDestroyed()) return;

        std::vector<ScriptComponent*> myScripts = owner->GetComponents<ScriptComponent>();
        for (ScriptComponent* sc : myScripts)
        {
            LifeScript* ls = dynamic_cast<LifeScript*>(sc->m_instance);
            if (ls) { ls->OnCollisionEnter(other); break; }
        }

        if (other->GetType() == Entity::TYPE::Player)
        {
            std::vector<ScriptComponent*> playerScripts = other->GetComponents<ScriptComponent>();
            for (ScriptComponent* sc : playerScripts)
            {
                LifeScript* ls = dynamic_cast<LifeScript*>(sc->m_instance);
                if (ls) { ls->TakeDamage(CollideDamage); break; }
            }
            isDead = true;
            ECS_ECS->DestroyEntity(owner);
        }

        if (other->GetType() == Entity::TYPE::Bullet)
        {
            ScriptComponent* bulletSc = other->GetComponent<ScriptComponent>();
            if (bulletSc && bulletSc->m_instance)
            {
                BulletScript* bs = dynamic_cast<BulletScript*>(bulletSc->m_instance);
                if (bs)
                {
                    dir = bs->GetOriginalFlyDirection();
                }
                else
                {
                    dir = other->transform.GetLocalForward();
                }
            }
            else
            {
                dir = other->transform.GetLocalForward();
            }

            XMINT4 random = { rand() % 100, rand() % 100, rand() % 100, rand() % 100 };
            rotation = { (float)random.x / 100.0f,(float)random.y / 100.0f,(float)random.z / 100.0f,(float)random.w / 100.0f };

            ToDestroy = true;
        }
    }

    END_SCRIPT(EnemyScript)

START_SCRIPT(EnemySpawner)
public:
Rail* spline = nullptr;
float  spawnInterval = 2.0f;
float  spawnAheadDistance = 30.0f;
RailFollower* railFollower = nullptr;
private:
float  m_timer = 0.f;
std::vector<Entity*> m_enemies;

void OnStart()
{
    m_timer = 0.f;
    m_enemies.clear();
}

void OnUpdate()
{
    if (!spline || !owner || owner->HasToBeDestroyed()) return;

    float dt = ECS_APP->GetTimer().DeltaTime();

    m_timer += dt;
    if (m_timer >= spawnInterval)
    {
        m_timer = 0.f;
        SpawnEnemy();
    }

    DirectX::XMFLOAT3 playerPos = owner->transform.GetWorldPosition();

    for (int i = 0; i < m_enemies.size(); i++)
    {
        Entity* e = m_enemies[i];
        if (!e || e->HasToBeDestroyed()) { m_enemies.erase(m_enemies.begin() + i); continue; }

        ScriptComponent* sc = e->GetComponent<ScriptComponent>();
        if (!sc || !sc->m_instance) { m_enemies.erase(m_enemies.begin() + i); continue; }

        EnemyScript* es = dynamic_cast<EnemyScript*>(sc->m_instance);
        if (!es || es->isDead) { m_enemies.erase(m_enemies.begin() + i); continue; }
    }

    for (Entity* e : m_enemies)
    {
        ScriptComponent* sc = e->GetComponent<ScriptComponent>();
        if (!sc || !sc->m_instance) continue;

        EnemyScript* es = dynamic_cast<EnemyScript*>(sc->m_instance);
        if (!es) continue;
        es->Update(playerPos);
    }

    for (int i = (int)m_enemies.size() - 1; i >= 0; --i)
        if (m_enemies[i] == nullptr)
            m_enemies.erase(m_enemies.begin() + i);
}

private:
    void SpawnEnemy()
    {
        if (!railFollower) return; 

        float spawnDist = railFollower->distance + spawnAheadDistance;

        DirectX::XMFLOAT3 spawnPos = spline->GetPositionAtDistance(spawnDist);
        DirectX::XMFLOAT3 tangent = spline->GetTangentAtDistance(spawnDist);

        float tLen = std::sqrt(tangent.x * tangent.x + tangent.y * tangent.y + tangent.z * tangent.z);
        if (tLen > 1e-4f) { tangent.x /= tLen; tangent.y /= tLen; tangent.z /= tLen; }

        DirectX::XMFLOAT3 up = { 0.f, 1.f, 0.f };
        DirectX::XMFLOAT3 right = {
            up.y * tangent.z - up.z * tangent.y,
            up.z * tangent.x - up.x * tangent.z,
            up.x * tangent.y - up.y * tangent.x
        };
        float rLen = std::sqrt(right.x * right.x + right.y * right.y + right.z * right.z);
        if (rLen > 1e-4f) { right.x /= rLen; right.y /= rLen; right.z /= rLen; }

        float lateral = (float)(rand() % 30); 
        float height = (float)(rand() % 10 + 2); 

        spawnPos.x += right.x * lateral;
        spawnPos.z += right.z * lateral;
        spawnPos.y += height;

        Entity* enemy = ECS_ECS->CreateEntity();
        enemy->transform.SetWorldPosition(spawnPos);

        ScriptComponent* sc = ECS_ECS->AddComponents<ScriptComponent>(enemy);
        sc->SetScript<EnemyScript>();

       // enemy->transform.WorldRotate({ 0,-90,0 });
        enemy->transform.LookAt(owner->transform.GetWorldPosition());

        m_enemies.push_back(enemy);
    }

END_SCRIPT(EnemySpawner)
