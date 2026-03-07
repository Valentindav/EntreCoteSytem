#pragma once
#include "Private/Component.h"
#include "Private/EngineCore.h"
#include "Private/Managers/ResourceManager.h"

#include <GpuTypes.h>
#include <string>

class UiImageComponent : public Component
{
public:
    TextureHandle m_textureHandle = kInvalidHandle;
    bool m_isVisible = true;

    float m_uv0X = 0.0f;
    float m_uv0Y = 0.0f;
    float m_uv1X = 1.0f;
    float m_uv1Y = 1.0f;

    UiImageComponent() = default;
    ~UiImageComponent() override = default;

    void LoadTexture(const std::string& filename)
    {
        m_textureHandle = ECS_ENGINE->GetResourceManager()->GetTextureHandle(filename);
    }

    virtual const ComponentType::Type GetType() override { return ComponentType::UiImage; }
};