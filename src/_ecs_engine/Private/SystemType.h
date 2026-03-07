#pragma once
#include <cstdint>

namespace SystemType
{
    enum Type : uint8_t
    {
        Physic,
        Camera,
        Script,
        Light,
        Render,
        Animation,
        Particle,
        UiRender,
        Button,
        Count
    };
}