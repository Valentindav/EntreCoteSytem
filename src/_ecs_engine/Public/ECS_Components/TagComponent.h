#pragma once
#include "Private/Component.h"
#include <string>

class TagComponent : public Component
{
public:
    std::string m_tag = "Untagged";

    TagComponent() = default;
    TagComponent(std::string tag) : m_tag(tag) {}

    virtual const ComponentType::Type GetType() override { return ComponentType::Tag; }
};