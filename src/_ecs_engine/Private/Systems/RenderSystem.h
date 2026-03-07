#pragma once
#include "System.h"
#include <vector>

class MeshComponent;

class RenderSystem : public System
{
public:
    RenderSystem() = default;
    ~RenderSystem() = default;

    void AddComponent(Component* _comp) override;
    void Update() override;

private:
    std::vector<MeshComponent*> m_meshes;
};