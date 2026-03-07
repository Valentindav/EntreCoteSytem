#pragma once
#include "d3dUtil.h"
#include <vector>
#include "UploadBuffer.h"
#include "Data/UiData.h" 

using namespace DirectX;
using Microsoft::WRL::ComPtr;


class UiManager
{
public:

	void Initialize(ID3D12Device* device, int maxVertices = 5000);

	std::unique_ptr<UploadBuffer<UIVertex>>& GetUIVertexBuffer() { return uiVertexBuffer; }
	void AddUIImage(float x, float y, float width, float height, int textureIndex,
		float u0 = 0.0f, float v0 = 0.0f, float u1 = 1.0f, float v1 = 1.0f);

	void AddUIText(std::string text, float x, float y, const FontData& font, int textureIndex);

	std::vector<UIVertex> GetUiVerticiesCPU() {
		return uiVerticesCPU;
	}
	

	int AddTextureToLists(ComPtr<ID3D12Resource>& Texture);


	void ClearUI();
	int getTextureCount() { return textureCount; }



private:

	int textureCount = 0;

	std::vector<ComPtr<ID3D12Resource>> uiTexturelist;

	std::unique_ptr<UploadBuffer<UIVertex>> uiVertexBuffer = nullptr;
	std::vector<UIVertex> uiVerticesCPU;


};

