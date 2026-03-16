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

	template<typename T>
	T* GetComponentInChildren();

	template<typename T>
	std::vector<T*> GetComponentsInChildren();

	template<typename T>
	T* GetComponentInParent();

	template<typename T>
	T* GetComponentInHierarchy();

	template<typename T>
	std::vector<T*> GetComponentsInHierarchy();

	bool HasParent() const { return parent != nullptr; }
	bool HasChildren() const { return !children.empty(); }
	Entity* GetParent() const { return parent; }
	Entity* GetRoot();
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
	return (EngineCore::GetInternalInstance())->GetECS()->GetComponent<T>(id);
}

template<typename T>
inline std::vector<T*> Entity::GetComponents()
{
	return (EngineCore::GetInternalInstance())->GetECS()->GetComponents<T>(id);
}

template<typename T>
inline T* Entity::GetComponentInChildren()
{
	T* comp = GetComponent<T>();
	if (comp != nullptr) return comp;

	for (Entity* child : children)
	{
		if (child != nullptr)
		{
			comp = child->GetComponentInChildren<T>();
			if (comp != nullptr) return comp;
		}
	}
	return nullptr;
}

template<typename T>
inline std::vector<T*> Entity::GetComponentsInChildren()
{
	std::vector<T*> result = GetComponents<T>();

	for (Entity* child : children)
	{
		if (child != nullptr)
		{
			std::vector<T*> childComps = child->GetComponentsInChildren<T>();
			result.insert(result.end(), childComps.begin(), childComps.end());
		}
	}
	return result;
}

template<typename T>
inline T* Entity::GetComponentInParent()
{
	T* comp = GetComponent<T>();
	if (comp != nullptr) return comp;

	if (parent != nullptr)
	{
		return parent->GetComponentInParent<T>();
	}

	return nullptr;
}

template<typename T>
inline T* Entity::GetComponentInHierarchy()
{
	Entity* root = GetRoot();

	return root->GetComponentInChildren<T>();
}

template<typename T>
inline std::vector<T*> Entity::GetComponentsInHierarchy()
{
	Entity* root = GetRoot();

	return root->GetComponentsInChildren<T>();
}