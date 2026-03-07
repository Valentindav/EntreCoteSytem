#include "Public/ECS_GameObjects/Camera.h"
#include "Public/ECS_Components/CameraComponent.h"
#include "Public/ECS.h"

#include "Private/EngineCore.h"

namespace Camera
{
	Entity* Camera(float viewX, float viewY, float viewW, float viewH, int RenderOrder)
	{
		ECS* ecs = ECS_ENGINE->GetECS();

		Entity* entity = ecs->CreateEntity();

		CameraComponent* cameraComp = ecs->AddComponents<CameraComponent>(entity);

		cameraComp->m_viewX = viewX;
		cameraComp->m_viewY = viewY;
		cameraComp->m_viewWidth = viewW;
		cameraComp->m_viewHeight = viewH;

		cameraComp->m_renderOrder = RenderOrder;

		return entity;
	}
}
