#pragma once
#include "Public/RaycastHit.h"

#include <DirectXMath.h>
#include <string>

class Physics
{
public:

    static bool Raycast(DirectX::XMFLOAT3 origin, DirectX::XMFLOAT3 direction, float maxDistance, RaycastHit& outHit, Entity* ignoreEntity = nullptr);
};