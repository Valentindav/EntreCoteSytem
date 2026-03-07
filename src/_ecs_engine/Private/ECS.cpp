#include "Public/ECS.h"
#include "Public/Entity.h"

#include "Private/Systems.h"

#include <algorithm>

using namespace DirectX;

ECS::ECS()
{
}

ECS::~ECS()
{
    for (auto* ent : m_entities) delete ent;
    for (auto& compList : m_components) {
        for (auto* comp : compList) delete comp;
    }
    for (auto* sys : m_systems) delete sys;

    m_entities.clear();
    m_components.clear();
}

void ECS::Init()
{
    m_currentSize = 0;
    m_entities.clear();
    m_entities.reserve(1024);

    m_components.clear();

	m_systems[(size_t)SystemType::Physic] = new PhysicSystem();
    m_systems[(size_t)SystemType::Camera] = new CameraSystem();
    m_systems[(size_t)SystemType::Script] = new ScriptSystem();
    m_systems[(size_t)SystemType::Particle] = new ParticleSystem();
    m_systems[(size_t)SystemType::Animation] = new AnimationSystem();
    m_systems[(size_t)SystemType::Light] = new LightSystem();
    m_systems[(size_t)SystemType::UiRender] = new UiRenderSystem();
    m_systems[(size_t)SystemType::Button] = new ButtonSystem();
    m_systems[(size_t)SystemType::Render] = new RenderSystem();
}

Entity* ECS::CreateEntity()
{
    Entity* newEntity = new Entity();
    newEntity->SetId(m_currentSize);
    newEntity->transform.mOwnerID = m_currentSize;
    newEntity->transform.SetOwner(newEntity);

    m_entities.push_back(newEntity);
    m_components.push_back(std::vector<Component*>());
    m_currentSize++;
    return newEntity;
}

Entity* ECS::GetEntity(int id)
{
    if (id >= 0 && id < m_entities.size()) {
        return m_entities[id];
    }
    return nullptr;
}

void ECS::DestroyEntity(Entity* entity)
{
    entity->toDestroy = true;
    m_entitiesToDestroy.push_back(entity);
}

void ECS::DestroyEntities()
{
    if (m_entitiesToDestroy.empty()) return;

    for (size_t i = 0; i < m_entitiesToDestroy.size(); ++i)
    {
        Entity* entity = m_entitiesToDestroy[i];

        for (Entity* child : entity->GetChildren())
        {
            if (!child->toDestroy) {
                DestroyEntity(child);
            }
        }
    }

    std::sort(m_entitiesToDestroy.begin(), m_entitiesToDestroy.end(), [](Entity* a, Entity* b) {
        return a->GetId() > b->GetId();
        });

    for (Entity* entity : m_entitiesToDestroy)
    {
        int entityId = entity->GetId();

        if (entityId < 0 || entityId >= m_entities.size()) continue;

        Entity* entityToDestroy = m_entities[entityId];
        int lastIndex = m_currentSize - 1;

        for (Component* comp : m_components[entityId])
        {
            delete comp;
        }

        if (entityId != lastIndex)
        {
            m_entities[entityId] = m_entities[lastIndex];
            m_entities[entityId]->SetId(entityId);

            m_components[entityId] = std::move(m_components[lastIndex]);

            for (Component* comp : m_components[entityId])
            {
                if (comp != nullptr)
                {
                    comp->mOwnerID = entityId;
                }
            }
        }

        m_entities.pop_back();
        m_components.pop_back();
        m_currentSize--;

        delete entityToDestroy;
    }

    m_entitiesToDestroy.clear();
}

void ECS::Update(const GameTimer& gt)
{
    for (int i = 0; i < m_components.size(); i++) {
        for (Component* comp : m_components[i]) {
            DristributeComponentToSystems(comp);
        }
	}

    for (auto* sys : m_systems)
    {
		if (sys == nullptr) continue;
        sys->Update();
    }

    DestroyEntities();
}

void ECS::DristributeComponentToSystems(Component* _comp)
{
	ComponentType::Type type = _comp->GetType();

    switch (type)
    {
    case ComponentType::Transform:
        break;
    case ComponentType::Camera:
		m_systems[SystemType::Camera]->AddComponent(_comp);
        break;
    case ComponentType::Mesh:
		m_systems[SystemType::Render]->AddComponent(_comp);
        break;
    case ComponentType::Animator:
        m_systems[SystemType::Animation]->AddComponent(_comp);
        break;
    case ComponentType::ParticleEmitter:
        m_systems[SystemType::Particle]->AddComponent(_comp);
        break;
    case ComponentType::RigidBody:
    case ComponentType::Collider:
        m_systems[SystemType::Physic]->AddComponent(_comp);
        break;
    case ComponentType::UiTransform:
        m_systems[SystemType::UiRender]->AddComponent(_comp);
        break;
    case ComponentType::UiButton:
        m_systems[SystemType::Button]->AddComponent(_comp);
        break;
    case ComponentType::Script:
        m_systems[SystemType::Script]->AddComponent(_comp);
        break;
    case ComponentType::Light:
        m_systems[SystemType::Light]->AddComponent(_comp);
        break;
    default:
        break;
    }
}
