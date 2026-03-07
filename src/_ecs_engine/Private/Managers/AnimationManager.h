#pragma once
#include <vector>
#include <DirectXMath.h>
#include <cstdint>

class AnimationManager
{
private:
    std::vector<DirectX::XMFLOAT4X4> m_frameBoneTransforms;

public:
    AnimationManager();
    ~AnimationManager() = default;

    void Clear();

    uint32_t SubmitBones(const std::vector<DirectX::XMFLOAT4X4>& boneTransforms);

    const std::vector<DirectX::XMFLOAT4X4>& GetFrameBoneTransforms() const;
};