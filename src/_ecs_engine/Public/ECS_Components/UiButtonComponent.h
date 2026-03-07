#pragma once
#include "Private/Component.h"
#include "Private/EngineCore.h"
#include "Private/Managers/ResourceManager.h"

#include <GpuTypes.h>
#include <string>
#include <vector>
#include <functional>

class UiButtonComponent : public Component
{
public:
    TextureHandle m_normalHandle = kInvalidHandle;
    TextureHandle m_hoverHandle = kInvalidHandle;
    std::vector<std::function<void()>> m_listeners;

    bool m_isVisible = false;

    UiButtonComponent() = default;
    ~UiButtonComponent() override = default;

    void LoadButtonStates(const std::string& normalFilename, const std::string& hoverFilename)
    {
        m_normalHandle = ECS_ENGINE->GetResourceManager()->GetTextureHandle(normalFilename);
        m_hoverHandle = ECS_ENGINE->GetResourceManager()->GetTextureHandle(hoverFilename);
        m_isVisible = true;
    }

    void AddListener(std::function<void()> listener)
    {
        m_listeners.push_back(listener);
    }

    virtual const ComponentType::Type GetType() override { return ComponentType::UiButton; }
};