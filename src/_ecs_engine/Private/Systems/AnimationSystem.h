#pragma once
#include "System.h"
#include <vector>

class AnimatorComponent;

class AnimationSystem : public System
{
private:
    std::vector<AnimatorComponent*> m_animators;

    void AddComponent(Component* _comp) override;
    virtual void Update() override;

    void UpdateAnimator(AnimatorComponent* _animator, float deltaTime);

public:
    AnimationSystem() = default;
    friend class ECS;
};