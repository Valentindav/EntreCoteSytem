#pragma once
#include "Private/Component.h"
#include "Private/EngineCore.h"
#include "Private/Managers/ResourceManager.h"

#include <GpuTypes.h>
#include <Data/UiData.h>
#include <Loaders/FontLoader.h>
#include <string>

class UiTextComponent : public Component
{
public:
    std::string m_text;
    FontData m_font;
    TextureHandle m_fontTextureHandle = kInvalidHandle;
    bool m_isVisible = true;

    UiTextComponent() = default;
    ~UiTextComponent() override = default;

    void LoadText(const std::string& text, const std::string& fontPng, const std::string& fontFnt)
    {
        m_text = text;

        m_fontTextureHandle = ECS_ENGINE->GetResourceManager()->GetTextureHandle(fontPng);

        m_font = FontLoader::LoadFont(fontFnt.c_str(), 512, 512);
    }

    virtual const ComponentType::Type GetType() override { return ComponentType::UiText; }
};