#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include <GameTimer.h>

#include "Private/SystemType.h"

class Component;
class System;
class Entity;

class ECS
{
private:

    int m_currentSize;

    std::vector<std::vector<Component*>> m_components;

    std::vector<Entity*> m_entities;
    std::vector<Entity*> m_entitiesToDestroy;
    System* m_systems[SystemType::Count];

    ECS();

    void Init();
    void DestroyEntities();
    void Update(const GameTimer& gt);
	void DristributeComponentToSystems(Component* _comp);
    
    template <typename T>
    void UpdateSystem(const GameTimer& gt);

public:

    ~ECS();

    Entity* CreateEntity();
    Entity* GetEntity(int id);
    void DestroyEntity(Entity* entity);

    int GetEntitiesSize() const { return (int)m_entities.size(); }
    const std::vector<Entity*>& GetEntities() const { return m_entities; }

    template <typename T>
    T* AddComponents(Entity* ent);

    template <typename T>
    std::vector<T*> GetComponents();

    template <typename T>
    std::vector<T*> GetComponents(int id);

    template <typename T>
    T* GetComponent(int id);

    friend class Entity;
    friend class EngineCore;
};

#include "Public/Entity.h"

template<typename T>
void ECS::UpdateSystem(const GameTimer& gt)
{
    for (int i = 0; i < m_components.size(); i++) {
        for (Component* comp : m_components[i]) {
            DristributeComponentToSystems(comp);
        }
    }

    for (auto& system : m_systems)
    {
        if (T* s = dynamic_cast<T*>(system))
        {
            s->Update();
            return;
        }
    }
}

template<typename T>
inline T* ECS::AddComponents(Entity* ent)
{
    static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");

    T* pNew = new T();
    pNew->mOwnerID = ent->GetId();

    if (ent->GetId() >= m_components.size()) {
        m_components.resize(ent->GetId() + 1);
    }

    m_components[ent->GetId()].push_back(pNew);
    return pNew;
}

template<typename T>
inline std::vector<T*> ECS::GetComponents()
{
    std::vector<T*> newcomp;
    for (auto& compList : m_components) {
        for (Component* comp : compList) {
            if (comp == nullptr) continue;
            T* castedComp = dynamic_cast<T*>(comp);
            if (castedComp != nullptr) {
                newcomp.push_back(castedComp);
            }
        }
    }
    return newcomp;
}

template<typename T>
inline std::vector<T*> ECS::GetComponents(int id)
{
    std::vector<T*> EntityComp;
    if (id >= m_components.size()) return EntityComp;

    for (Component* comp : m_components[id]) {
        T* castedComp = dynamic_cast<T*>(comp);
        if (castedComp != nullptr) {
            EntityComp.push_back(castedComp);
        }
    }
    return EntityComp;
}

template<typename T>
inline T* ECS::GetComponent(int id)
{
    if (id >= m_components.size()) return nullptr;

    for (Component* comp : m_components[id]) {
        if (comp == nullptr) continue;
        T* castedComp = dynamic_cast<T*>(comp);
        if (castedComp != nullptr) {
            return castedComp;
        }
    }
    return nullptr;
}