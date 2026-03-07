#pragma once
#include "private/Component.h"
#include "Private/NativeScript.h"

class ScriptComponent : public Component
{
public:
    NativeScript* m_instance = nullptr;

    ScriptComponent() = default;

    virtual ~ScriptComponent() {
        if (m_instance) {
            delete m_instance;
            m_instance = nullptr;
        }
    }

    template<typename T>
    void SetScript()
    {
        if (m_instance) {
            delete m_instance;
        }

        m_instance = new T();

        if (m_instance) {
            m_instance->owner = GetOwner();
        }
    }

    virtual const ComponentType::Type GetType() override { return ComponentType::Script; }

private:
    bool m_started = false;
    bool m_wasColliding = false;
    bool m_isColliding = false;

    friend class ScriptSystem;
    friend class PhysicSystem;

};