#include "Public/ECS_GameObjects/ParticleEmitter.h"

#include "Private/EngineCore.h"
#include "Public/ECS.h"

#include "Public/ECS_Components/ParticleEmitterComponent.h"

namespace ParticleEmmiter
{
	Entity* ParticleEmmiter()
	{
		ECS* ecs = ECS_ENGINE->GetECS();

		Entity* entity = ecs->CreateEntity();
		
		ParticleEmitterComponent* particleEmitterComp = ecs->AddComponents<ParticleEmitterComponent>(entity);

		return entity;
	}
}

