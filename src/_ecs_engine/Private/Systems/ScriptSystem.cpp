#include "ScriptSystem.h"
#include <GameTimer.h>
#include "Public/ECS_Components/ScriptComponent.h"
#include "Public/ECS_Components/ColliderComponent.h"
#include "Public/Entity.h"
#include "Private/NativeScript.h"
#include <algorithm>

void ScriptSystem::AddComponent(Component* _comp)
{
    m_scripts.push_back(static_cast<ScriptComponent*>(_comp));
}

void ScriptSystem::Update()
{
    for (ScriptComponent* comp : m_scripts)
    {
        if (comp->m_instance == nullptr) continue;

        if (comp->m_started == false)
        {
            comp->m_instance->OnStart();
            comp->m_started = true;
        }

        comp->m_instance->OnUpdate();

        if (comp->GetOwner()->HasToBeDestroyed())
        {
            comp->m_instance->OnDestroy();
        }

        ColliderComponent* collider = comp->GetOwner()->GetComponent<ColliderComponent>();
        if (collider == nullptr) {
            collider = comp->GetOwner()->GetComponentInHierarchy<ColliderComponent>();
        }  

        if (collider)
        {
            const std::vector<Entity*>& currentCollisions = collider->m_entitiesCollided;
            std::vector<Entity*>& lastCollisions = comp->m_lastEntitiesCollided;

            for (Entity* otherEntity : currentCollisions)
            {
                if (std::find(lastCollisions.begin(), lastCollisions.end(), otherEntity) == lastCollisions.end())
                {
                    comp->m_instance->OnCollisionEnter(otherEntity);
                }
                else
                {
                    comp->m_instance->OnCollision(otherEntity);
                }
            }

            for (Entity* oldEntity : lastCollisions)
            {
                if (std::find(currentCollisions.begin(), currentCollisions.end(), oldEntity) == currentCollisions.end())
                {
                    comp->m_instance->OnCollisionExit(oldEntity);
                }
            }

            comp->m_lastEntitiesCollided = currentCollisions;
        }
    }

    m_scripts.clear();
}