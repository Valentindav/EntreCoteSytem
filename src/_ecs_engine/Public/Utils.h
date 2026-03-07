#pragma once
#include <DirectXMath.h>
#include <string>

namespace Utils {
	DirectX::XMVECTOR ScreenToWorld(DirectX::XMFLOAT2 mousePos);

	void SwitchMenu(std::string menu);
}