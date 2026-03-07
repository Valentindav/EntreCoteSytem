#include "UiRenderSystem.h"

#include "Public/ECS_Components/UiTransformComponent.h"
#include "Public/ECS_Components/UiImageComponent.h"
#include "Public/ECS_Components/UiTextComponent.h"
#include "Public/ECS_Components/UiAnimatorComponent.h"
#include "Public/Entity.h"
#include "Public/ECS.h"

#include "Private/Managers/UiManager.h"
#include "Private/EngineCore.h"

void UiRenderSystem::AddComponent(Component* _comp)
{
    m_uiTransforms.push_back(static_cast<UiTransformComponent*>(_comp));
}

void UiRenderSystem::Update()
{
    if (m_uiTransforms.empty()) return;

    float deltaTime = ECS_ENGINE->GetTimer().DeltaTime();
    float winW = (float)ECS_ENGINE->GetClientWidth();
    float winH = (float)ECS_ENGINE->GetClientHeight();

    UiManager* uiManager = ECS_ENGINE->GetUiManager();
    ECS* ecs = ECS_ENGINE->GetECS();

    if (!uiManager) {
        m_uiTransforms.clear();
        return;
    }

    std::sort(m_uiTransforms.begin(), m_uiTransforms.end(), [](const UiTransformComponent* pA, const UiTransformComponent* pB) { return pA->m_depth < pB->m_depth; });

    for (UiTransformComponent* transform : m_uiTransforms)
    {
        Entity* ent = ecs->GetEntity(transform->mOwnerID);
        if (!ent) continue;

        auto* imageComp = ecs->GetComponent<UiImageComponent>(ent->GetId());
        auto* textComp = ecs->GetComponent<UiTextComponent>(ent->GetId());
        auto* animComp = ecs->GetComponent<UiAnimatorComponent>(ent->GetId());

        float absX = transform->m_x * winW;
        float absY = transform->m_y * winH;
        float absW = transform->m_width * winW;
        float absH = transform->m_height * winH;

        if (imageComp && imageComp->m_isVisible)
        {
            if (animComp && animComp->m_isPlaying)
            {
                animComp->m_timer += deltaTime;
                float frameDuration = 1.0f / animComp->m_framesPerSecond;

                if (animComp->m_timer >= frameDuration) {
                    animComp->m_timer -= frameDuration;
                    animComp->m_currentFrame++;

                    if (animComp->m_currentFrame >= animComp->m_maxFrames) {
                        if (animComp->m_loop) animComp->m_currentFrame = 0;
                        else { animComp->m_currentFrame = animComp->m_maxFrames - 1; animComp->m_isPlaying = false; }
                    }
                }

                int textureWidth = animComp->m_maxFrames * animComp->m_height;
                int textureHeight = animComp->m_width;

                int cols = textureWidth / animComp->m_height;

                int currentColumn = animComp->m_currentFrame % cols;
                int currentRow = animComp->m_currentFrame % cols;

                imageComp->m_uv0X = (currentColumn * animComp->m_height) / (float)textureWidth;
                imageComp->m_uv0Y = (currentRow * animComp->m_width) / (float)textureHeight;
                imageComp->m_uv1X = ((currentColumn + 1) * animComp->m_height) / (float)textureWidth;
                imageComp->m_uv1Y = ((currentRow + 1) * animComp->m_width) / (float)textureHeight;
            }

            uiManager->AddUIImage(absX, absY, absW, absH, imageComp->m_textureHandle,
                imageComp->m_uv0X, imageComp->m_uv0Y,
                imageComp->m_uv1X, imageComp->m_uv1Y);
        }

        if (textComp && textComp->m_isVisible && !textComp->m_text.empty())
        {
            uiManager->AddUIText(textComp->m_text, absX, absY, textComp->m_font, textComp->m_fontTextureHandle);
        }
    }

    m_uiTransforms.clear();
}