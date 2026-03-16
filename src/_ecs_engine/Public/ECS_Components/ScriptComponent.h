#pragma once
#include "Private/Component.h"
#include "Private/NativeScript.h"
#include <vector>
#include <algorithm>

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
    T* SetScript()
    {
        if (m_instance) {
            delete m_instance;
        }
        m_instance = new T();
        if (m_instance) {
            m_instance->owner = GetOwner();
        }
        return dynamic_cast<T*>(m_instance);
    }

    template<typename T>
    T* GetScript()
    {
        return dynamic_cast<T*>(m_instance);
    }

    NativeScript* GetScript()
    {
        return m_instance;
    }

    template<typename T>
    bool IsScript()
    {
        if (m_instance == nullptr) {
            return false;
        }
        if (dynamic_cast<T*>(m_instance) == nullptr) {
            return false;
        }
        return true;
    }

    virtual const ComponentType::Type GetType() override { return ComponentType::Script; }

private:
    bool m_started = false;

    std::vector<Entity*> m_lastEntitiesCollided;

    friend class ScriptSystem;
};