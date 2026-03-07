#include "Public/Utils.h"
#include "Private/EngineCore.h"
#include "Private/Managers/CameraManager.h"
#include "Private/Managers/MenuManager.h"

#include <RenderCamera.h>

namespace Utils {

	DirectX::XMVECTOR ScreenToWorld(DirectX::XMFLOAT2 mousePos) {
		float ndcX = (2.0f * mousePos.x) / ECS_ENGINE->GetClientWidth() - 1.0f;
		float ndcY = 1.0f - (2.0f * mousePos.y) / ECS_ENGINE->GetClientHeight();

		DirectX::XMVECTOR ndcVec = DirectX::XMVectorSet(ndcX, ndcY, 0.0f, 1.0f);

		DirectX::XMMATRIX viewProj = ECS_ENGINE->GetCameraManager()->GetRenderCameras()[0]->GetView() * ECS_ENGINE->GetCameraManager()->GetRenderCameras()[0]->GetProj();
		DirectX::XMMATRIX invViewProj = DirectX::XMMatrixInverse(nullptr, viewProj);

		DirectX::XMVECTOR worldPos = DirectX::XMVector3TransformCoord(ndcVec, invViewProj);
		return worldPos;
	}

	void SwitchMenu(std::string menu)
	{
		ECS_ENGINE->GetMenuManager()->switchMenu(menu);
	}
}