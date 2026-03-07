#include "AnimationSystem.h"

#include <Data/AnimationData.h>

#include "Private/EngineCore.h"
#include "Private/Managers/ResourceManager.h"
#include "Private/Managers/AnimationManager.h"
#include "Public/ECS_Components/AnimatorComponent.h"

void AnimationSystem::AddComponent(Component* _comp)
{
    m_animators.push_back(static_cast<AnimatorComponent*>(_comp));
}

void AnimationSystem::Update()
{
    if (m_animators.empty()) return;

    float deltaTime = ECS_ENGINE->GetTimer().DeltaTime();

    for (AnimatorComponent* animator : m_animators)
    {
        if (animator->IsPlaying)
        {
            UpdateAnimator(animator, deltaTime);
        }
    }
    m_animators.clear(); 
}

void AnimationSystem::UpdateAnimator(AnimatorComponent* _animator, float deltaTime)
{
    SkeletonHandle handle = _animator->GetSkeletonHandle();
    const SkeletonData* data = ECS_ENGINE->GetResourceManager()->GetSkeletonData(handle);

    if (!data || _animator->CurrentClipName.empty()) return;

    _animator->TimePos += deltaTime * _animator->PlaybackSpeed;

    float duration = data->Animations.at(_animator->CurrentClipName).GetClipEndTime();
    if (_animator->TimePos > duration) {
        if (_animator->Loop) _animator->TimePos = fmod(_animator->TimePos, duration);
        else _animator->TimePos = duration;
    }

    data->GetFinalTransforms(
        _animator->CurrentClipName,
        _animator->TimePos,
        _animator->FinalTransforms
    );

    _animator->CurrentBoneOffset = ECS_ENGINE->GetAnimationManager()->SubmitBones(_animator->FinalTransforms);
}