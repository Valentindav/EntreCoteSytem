#pragma once
#include <DirectXMath.h>
#include "Helpers/MathHelper.h"

using namespace DirectX;

struct InstanceData
{
    DirectX::XMFLOAT4 Color;

    DirectX::XMFLOAT4 World0;
    DirectX::XMFLOAT4 World1;
    DirectX::XMFLOAT4 World2;
    DirectX::XMFLOAT4 World3;

    UINT TexIndex;
    UINT MatIndex;
    UINT BoneOffset;
    UINT Pad;
};