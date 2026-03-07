#pragma once
#include <cstdint>

using MeshHandle = uint32_t;
using TextureHandle = uint32_t;
using MaterialHandle = uint32_t;
using SkeletonHandle = uint32_t;

static const uint32_t kInvalidHandle = 0xFFFFFFFF;

enum RenderFlags : char
{
    None = 0,
    Solid = 1 << 0,     // 0b0001
    Wireframe = 1 << 1  // 0b0010
};