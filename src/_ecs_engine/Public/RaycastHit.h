#pragma once
#include "Public/Entity.h"

#include <DirectXMath.h>

struct RaycastHit
{
    bool hit;
    Entity* entity;
    DirectX::XMFLOAT3 point;
    DirectX::XMFLOAT3 normal;
    float distance;
};