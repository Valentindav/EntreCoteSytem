#pragma once
#include "d3dUtil.h"
using namespace DirectX;
using Microsoft::WRL::ComPtr;

class TextureUI
{
public: 
	ComPtr<ID3D12Resource> Texture;
	ComPtr<ID3D12Resource> TextureUploadHeap;
};

