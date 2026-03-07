#pragma once
#include "Private/Component.h"
#include "Private/Managers/ResourceManager.h"
#include "Private/EngineCore.h"

#include <DirectXMath.h>
#include "GpuTypes.h"
#include <string>

using namespace DirectX;

class MaterialComponent : public Component
{
public:

    TextureHandle m_textureHandle = 0;

    MaterialHandle m_materialHandle = 0;

    XMFLOAT4 m_diffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };

    XMFLOAT3 FresnelR0 = { 0.04f, 0.04f, 0.04f };

    float Roughness = 0.5f;
    float Metalness = 0.f;

    std::string m_materialName = "default";
    std::string m_textureName = "default";

    void LoadTexture(const std::string& filename)
    {
        m_textureName = filename;

        m_textureHandle = ECS_ENGINE->GetResourceManager()->GetTextureHandle(filename);
    }

    void SetColor(DirectX::XMFLOAT4 col) { m_diffuseAlbedo = col; }
    DirectX::XMFLOAT4 GetColor() { return m_diffuseAlbedo; }

    MaterialComponent() = default;

    MaterialComponent(uint32_t textureIdx, XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f })
        : m_textureHandle(textureIdx), m_diffuseAlbedo(color) {
    }

    virtual const ComponentType::Type GetType() { return ComponentType::Material; }
};

