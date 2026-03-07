#pragma once
#include <DirectXMath.h>
#include "Helpers/MathHelper.h"

using namespace DirectX;

struct LightData
{
    int               Type;
    DirectX::XMFLOAT3 Color;
    DirectX::XMFLOAT3 Position;
    float             Range;
    DirectX::XMFLOAT3 Direction;
    float             SpotInnerAngle;
    DirectX::XMFLOAT3 Attenuation;
    float             SpotOuterAngle;
};