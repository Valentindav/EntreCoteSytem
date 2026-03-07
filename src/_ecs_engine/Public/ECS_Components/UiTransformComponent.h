#pragma once
#include "Private/Component.h"

class UiTransformComponent : public Component
{
public:
    float m_x = 0.0f;
    float m_y = 0.0f;
    float m_width = 0.0f;
    float m_height = 0.0f;
    float m_depth = 0.0f;

    UiTransformComponent() = default;
    ~UiTransformComponent() override = default;

    void SetRect(float x, float y, float w, float h)
    {
        m_x = x; m_y = y; m_width = w; m_height = h;
    }

    virtual const ComponentType::Type GetType() override { return ComponentType::UiTransform; }
};