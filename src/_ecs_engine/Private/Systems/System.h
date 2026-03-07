#pragma once

class Component;

class System
{
public:

	virtual ~System() = default;

	virtual void AddComponent(Component* _comp) = 0;
	virtual void Update() = 0;

	friend class ECS;
};

