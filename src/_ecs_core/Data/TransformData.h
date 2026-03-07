#pragma once
#include <DirectXMath.h>
using namespace DirectX;
struct Transform
{
    XMFLOAT3 position{ 0, 0, 0 };

    XMFLOAT3 scale{ 1, 1, 1 };

    XMFLOAT3 right  { 1, 0, 0 };
    XMFLOAT3 up     { 0, 1, 0 };
    XMFLOAT3 forward{ 0, 0, 1 };

    XMFLOAT4 rotation = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

};