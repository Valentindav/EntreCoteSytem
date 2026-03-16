#pragma once

#include <cstdint>

namespace ComponentType
{
	enum Type : uint8_t
	{
		Transform,
		Camera,
		Mesh,
		RigidBody,
		Collider,
		Material,
		UiTransform,
		UiImage,
		UiButton,
		UiText,
		UiAnimator,
		Script,
		Light,
		Animator,
		ParticleEmitter,
		Tag,
		Count
	};
}

class Entity;
class ECS;

class Component
{
public:
	int mOwnerID = -1;

	virtual ~Component() = default;

	int GetOwnerID() const { return mOwnerID; }
	virtual const ComponentType::Type GetType() = 0;

	Entity* GetOwner();
};