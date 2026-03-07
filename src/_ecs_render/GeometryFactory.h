#pragma once
#include "Data/MeshData.h"
#include <DirectXMath.h>

namespace Geometry
{
    MeshData CreateBox(float width, float height, float depth, uint32_t numSubdivisions);

    MeshData CreateSphere(float radius, uint32_t sliceCount, uint32_t stackCount);

    MeshData CreateGeosphere(float radius, uint32_t numSubdivisions);

    MeshData CreateCylinder(float bottomRadius, float topRadius, float height, uint32_t sliceCount, uint32_t stackCount);

    MeshData CreateGrid(float width, float depth, uint32_t m, uint32_t n);

    MeshData CreateQuad(float x, float y, float w, float h, float depth);
}