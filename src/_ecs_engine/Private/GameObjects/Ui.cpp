#include "Public/ECS_GameObjects/Ui.h"

#include "Private/EngineCore.h"

#include "Public/ECS.h"
#include "Public/ECS_Components/UiTransformComponent.h"
#include "Public/ECS_Components/UiImageComponent.h"
#include "Public/ECS_Components/UiTextComponent.h"
#include "Public/ECS_Components/UiButtonComponent.h"

namespace Ui
{
	Entity* Image(float x, float y, float w, float h, std::string filePath)
	{
		ECS* ecs = ECS_ENGINE->GetECS();

		Entity* entity = ecs->CreateEntity();

		UiTransformComponent* transform = ecs->AddComponents<UiTransformComponent>(entity);
		transform->SetRect(x, y, w, h);

		UiImageComponent* image = ecs->AddComponents<UiImageComponent>(entity);
		image->LoadTexture(filePath);

		return entity;
	}

	Entity* Text(float x, float y, std::string text)
	{
		ECS* ecs = ECS_ENGINE->GetECS();

		Entity* entity = ecs->CreateEntity();

		UiTransformComponent* transform = ecs->AddComponents<UiTransformComponent>(entity);
		transform->SetRect(x, y, 0.f, 0.f);

		UiTextComponent* textComp = ecs->AddComponents<UiTextComponent>(entity);
		textComp->LoadText(text, "../../res/Arial_0.png", "../../res/Arial.fnt");

		return entity;
	}

	Entity* Button(float x, float y, float w, float h, std::string normalFilePath, std::string hoverFilePath)
	{
		ECS* ecs = ECS_ENGINE->GetECS();

		Entity* entity = ecs->CreateEntity();

		UiTransformComponent* transform = ecs->AddComponents<UiTransformComponent>(entity);
		transform->SetRect(x, y, w, h);

		UiButtonComponent* button = ecs->AddComponents<UiButtonComponent>(entity);
		button->LoadButtonStates(normalFilePath, hoverFilePath);

		return entity;
	}
}

