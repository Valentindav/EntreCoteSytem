#pragma once
#include "private/Component.h"

#include <GpuTypes.h>
#include <DirectXMath.h>
#include <string>
#include <vector>

#include "Private/Managers/ResourceManager.h"

class AnimatorComponent : public Component
{
private:
    // Moteur
    SkeletonHandle m_skeletonHandle = kInvalidHandle;
    std::string m_skeletonName = "default";

public:

    std::string CurrentClipName;
    float TimePos = 0.0f;
    bool IsPlaying = true;
    bool Loop = true;
    float PlaybackSpeed = 1.0f;

    std::vector<DirectX::XMFLOAT4X4> FinalTransforms;
    uint32_t CurrentBoneOffset = 0xFFFFFFFF;

    AnimatorComponent() = default;
    ~AnimatorComponent() override = default;

    void LoadSkeleton(const std::string& filename)
    {
        m_skeletonName = filename;

        m_skeletonHandle = ECS_ENGINE->GetResourceManager()->GetSkeletonHandle(filename);
    }

    void Play(const std::string& clipName, bool loop = true, float speed = 1.0f)
    {
        CurrentClipName = clipName;
        Loop = loop;
        PlaybackSpeed = speed;
        IsPlaying = true;
        TimePos = 0.0f;
    }

    void Stop()
    {
        PlaybackSpeed = 0;
        IsPlaying = false;
        Loop = false;
        TimePos = 0;
        CurrentBoneOffset = 0;
    }

    SkeletonHandle GetSkeletonHandle() const { return m_skeletonHandle; }

    virtual const ComponentType::Type GetType() { return ComponentType::Animator; }

    friend class EngineCore;
    friend class AnimationSystem;
    friend class RenderSystem;
};