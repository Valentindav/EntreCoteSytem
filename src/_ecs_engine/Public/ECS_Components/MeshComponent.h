#pragma once
#include "private/Component.h"
#include "Private/EngineCore.h" 
#include "Private/Managers/ResourceManager.h"

#include <GpuTypes.h>
#include <DirectXMath.h>
#include <string>

class MeshComponent : public Component
{
private:

    // moteur
    MeshHandle m_meshHandle = kInvalidHandle;

	// gameplay
    DirectX::XMFLOAT4 m_color = { 1.0f, 1.0f, 1.0f, 1.0f };
    bool m_isVisible = true;

    std::string m_meshName = "default";


public:
    MeshComponent() = default;
    ~MeshComponent() override = default;

    void LoadMesh(const std::string& filename)
    {
        m_meshName = filename;

        m_meshHandle = EngineCore::GetInternalInstance()->GetResourceManager()->GetMeshHandle(filename);
    }

    void SetColor(DirectX::XMFLOAT4 col) { m_color = col; }
    DirectX::XMFLOAT4 GetColor() const { return m_color; }

    void SetVisible(bool visible) { m_isVisible = visible; }
    bool IsVisible() const { return m_isVisible; }

    MeshHandle GetMeshHandle() const { return m_meshHandle; }
    void SetMeshHandle(MeshHandle handle) { m_meshHandle = handle; }

	std::string GetMeshName() const { return m_meshName; }

    virtual const ComponentType::Type GetType() { return ComponentType::Mesh; }

    friend class EngineCore;
	friend class RenderSystem;
};  