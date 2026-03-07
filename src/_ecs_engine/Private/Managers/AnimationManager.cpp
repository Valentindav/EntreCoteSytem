#include "AnimationManager.h"

AnimationManager::AnimationManager()
{
    m_frameBoneTransforms.reserve(10000);
}

void AnimationManager::Clear()
{
    m_frameBoneTransforms.clear();
}

uint32_t AnimationManager::SubmitBones(const std::vector<DirectX::XMFLOAT4X4>& boneTransforms)
{
    if (boneTransforms.empty())
    {
        return 0xFFFFFFFF;
    }

    uint32_t offset = static_cast<uint32_t>(m_frameBoneTransforms.size());

    m_frameBoneTransforms.insert(m_frameBoneTransforms.end(), boneTransforms.begin(), boneTransforms.end());

    return offset;
}

const std::vector<DirectX::XMFLOAT4X4>& AnimationManager::GetFrameBoneTransforms() const
{
    return m_frameBoneTransforms;
}