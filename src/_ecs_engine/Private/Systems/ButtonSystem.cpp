#include "ButtonSystem.h"

#include "Public/ECS_Components/UiButtonComponent.h"
#include "Public/ECS_Components/UiTransformComponent.h"
#include "Public/ECS_Components/UiImageComponent.h"
#include "Public/Inputs.h"
#include "Public/Entity.h"
#include "Public/ECS.h"
#include "Private/EngineCore.h"

void ButtonSystem::AddComponent(Component* _comp)
{
    m_buttons.push_back(static_cast<UiButtonComponent*>(_comp));
}

void ButtonSystem::Update()
{
    if (m_buttons.empty()) return;

    // 1. Récupérer les inputs une seule fois par frame
    auto mousePos = Inputs::GetMousePosition();
    bool isMouseClicked = Inputs::IsMouseDown(Mouse::LEFT);

    float winW = (float)ECS_ENGINE->GetClientWidth();
    float winH = (float)ECS_ENGINE->GetClientHeight();

    ECS* ecs = ECS_ENGINE->GetECS();

    for (UiButtonComponent* buttonComp : m_buttons)
    {
        if (!buttonComp->m_isVisible) continue;

        Entity* ent = ecs->GetEntity(buttonComp->mOwnerID);
        if (!ent) continue;

        auto* transform = ecs->GetComponent<UiTransformComponent>(ent->GetId());
        auto* imageComp = ecs->GetComponent<UiImageComponent>(ent->GetId());

        if (!transform || !imageComp) continue;

        float absX = transform->m_x * winW;
        float absY = transform->m_y * winH;
        float absW = transform->m_width * winW;
        float absH = transform->m_height * winH;

        if (mousePos.x > absX && mousePos.x < absX + absW &&
            mousePos.y > absY && mousePos.y < absY + absH)
        {
            imageComp->m_textureHandle = buttonComp->m_hoverHandle;

            if (isMouseClicked) {
                for (auto& listener : buttonComp->m_listeners) {
                    if (listener) listener();
                }
            }
        }
        else
        {
            imageComp->m_textureHandle = buttonComp->m_normalHandle;
        }
    }

    m_buttons.clear();
}