#pragma once
#include <DirectXMath.h>

class Entity;

namespace Light
{
	Entity* Point(float range, DirectX::XMFLOAT3 attenuation = { 1.f, 0.1f, 0.01f }, DirectX::XMFLOAT3 color = { 1.f, 1.f, 1.f }, float intensity = 1.f);
	Entity* Directional(DirectX::XMFLOAT3 direction, DirectX::XMFLOAT3 color = { 1.f, 1.f, 1.f }, float intensity = 1.f);
	Entity* Spot(DirectX::XMFLOAT3 direction, float range, float innerAngle, float outerAngle, DirectX::XMFLOAT3 attenuation = { 1.f, 0.1f, 0.01f }, DirectX::XMFLOAT3 color = { 1.f, 1.f, 1.f }, float intensity = 1.f);
}