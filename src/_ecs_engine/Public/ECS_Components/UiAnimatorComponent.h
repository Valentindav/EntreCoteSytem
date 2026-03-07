#pragma once
#include "Private/Component.h"

class UiAnimatorComponent : public Component
{
public:
    int m_width = 1;
    int m_height = 1;

    int m_currentFrame = 0;
    int m_maxFrames = 1;

    float m_framesPerSecond = 12.0f;
    float m_timer = 0.0f;

    bool m_isPlaying = true;
    bool m_loop = true;

    UiAnimatorComponent() = default;
    ~UiAnimatorComponent() override = default;

    void Play(int columns, int rows, float fps, int maxFrames = -1)
    {
        m_width = columns;
        m_height = rows;
        m_framesPerSecond = fps;
        m_maxFrames = (maxFrames > 0) ? maxFrames : (columns * rows);
    }

    virtual const ComponentType::Type GetType() override { return ComponentType::UiAnimator; }
};