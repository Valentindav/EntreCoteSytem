#include "Public/ECS_GameObjects/Geometry.h"

#include "Private/EngineCore.h"
#include "Public/ECS.h"

#include "Public/ECS_Components/MeshComponent.h"

namespace Geometry
{
	Entity* Cube()
	{
		ECS* ecs = ECS_ENGINE->GetECS();

		Entity* entity = ecs->CreateEntity();

		MeshComponent* mesh = ecs->AddComponents<MeshComponent>(entity);
		mesh->LoadMesh("cube");

		return entity;
	}

	Entity* Sphere()
	{
		ECS* ecs = ECS_ENGINE->GetECS();

		Entity* entity = ecs->CreateEntity();

		MeshComponent* mesh = ecs->AddComponents<MeshComponent>(entity);
		mesh->LoadMesh("sphere");

		return entity;
	}

	Entity* Cylender()
	{
		ECS* ecs = ECS_ENGINE->GetECS();

		Entity* entity = ecs->CreateEntity();

		MeshComponent* mesh = ecs->AddComponents<MeshComponent>(entity);
		mesh->LoadMesh("cylender");

		return entity;
	}
}

