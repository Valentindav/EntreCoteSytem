#include "Public/ECS_GameObjects/Lights.h"

#include "Private/EngineCore.h"
#include "Public/ECS.h"

#include "Public/ECS_Components/LightComponent.h"

namespace Light
{
	Entity* Point(float range, DirectX::XMFLOAT3 attenuation, DirectX::XMFLOAT3 color, float intensity)
	{
		ECS* ecs = ECS_ENGINE->GetECS();

		Entity* entity = ecs->CreateEntity();
		
		LightComponent* lightComp = ecs->AddComponents<LightComponent>(entity);
		lightComp->SetAsPoint(range, attenuation, color, intensity);

		return entity;
	}

	Entity* Directional(DirectX::XMFLOAT3 direction, DirectX::XMFLOAT3 color, float intensity)
	{
		ECS* ecs = ECS_ENGINE->GetECS();

		Entity* entity = ecs->CreateEntity();
		
		LightComponent* lightComp = ecs->AddComponents<LightComponent>(entity);
		lightComp->SetAsDirectional(direction, color, intensity);

		return entity;
	}

	Entity* Spot(DirectX::XMFLOAT3 direction, float range, float innerAngle, float outerAngle, DirectX::XMFLOAT3 attenuation, DirectX::XMFLOAT3 color, float intensity)
	{
		ECS* ecs = ECS_ENGINE->GetECS();

		Entity* entity = ecs->CreateEntity();
		
		LightComponent* lightComp = ecs->AddComponents<LightComponent>(entity);
		lightComp->SetAsSpot(direction, range, innerAngle, outerAngle, attenuation, color, intensity);

		return entity;
	}
}

