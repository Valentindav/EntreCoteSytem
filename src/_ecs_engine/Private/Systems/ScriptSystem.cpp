#include "ScriptSystem.h"
#include <GameTimer.h>
#include "Public/ECS_Components/ScriptComponent.h"
#include "Public/Entity.h"

#include "Private/NativeScript.h"

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

        if (comp->m_isColliding)
        {
            if (comp->m_wasColliding == false)
            {
                comp->m_instance->OnCollisionEnter();
            }
            else
            {
                comp->m_instance->OnCollision();
            }
        }
        else
        {
            if (comp->m_wasColliding == true)
            {
                comp->m_instance->OnCollisionExit();
            }
        }
        comp->m_wasColliding = comp->m_isColliding;
        comp->m_isColliding = false;
  
    }

    m_scripts.clear();
}