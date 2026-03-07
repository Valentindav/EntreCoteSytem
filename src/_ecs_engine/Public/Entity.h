#pragma once
#include "Public/ECS_Components/TransformComponent.h"
#include <vector>

class ECS;

class Entity
{
	int id = 0;
	bool toDestroy = false;

	Entity* parent = nullptr;
	std::vector<Entity*> children;

public:
	enum TYPE {
		Bullet,
		Player,
		Ennemy,
		Ui,
		Other,

		Count
	};

	TYPE type = TYPE::Other;
	TransformComponent transform;

	int GetId() const { return id; }
	void SetId(int newid) { id = newid; transform.mOwnerID = newid; }
	
	bool HasToBeDestroyed() const { return toDestroy; }

	template<typename T>
	T* GetComponent();

	template<typename T>
	std::vector<T*> GetComponents();

	bool HasParent() const { return parent != nullptr; }
	bool HasChildren() const { return !children.empty(); }
	Entity* GetParent() const { return parent; }
	std::vector<Entity*> GetChildren() const { return children; }
	void SetParent(Entity* newParent);

	void RemoveParent();

	void AddChild(Entity* child);

	void RemoveChild(Entity* child);

	void SetType(TYPE _type) { type = _type; };

	TYPE GetType() { return type; }

	friend class ECS;
};

#include "Public/ECS.h"
#include "Private/EngineCore.h"

template<typename T>
inline T* Entity::GetComponent()
{
	return ECS_ENGINE->GetECS()->GetComponent<T>(id);
}

template<typename T>
inline std::vector<T*> Entity::GetComponents()
{
	return ECS_ENGINE->GetECS()->GetComponents<T>(id);
}

inline void Entity::SetParent(Entity* parent) { parent->AddChild(this); }

inline void Entity::RemoveParent() { parent->RemoveChild(this); }