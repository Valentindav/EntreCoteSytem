#pragma once
#include "d3dUtil.h"
#include <DirectXMath.h>

class RenderCamera
{
public:
    RenderCamera(int clientWidth, int clientHeight);
    ~RenderCamera();

    void SetScissorRect(int left, int top, int right, int bottom);
    void ApplyViewport(ID3D12GraphicsCommandList* cmdList) const;

    void SetViewport(float width, float height, float topLeftX = 0.0f, float topLeftY = 0.0f, float minDepth = 0.0f, float maxDepth = 1.0f);
    void SetLens(float fovY, float aspect, float nearZ, float farZ);

    void LookAt(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up);

    DirectX::XMMATRIX GetView() const;

    DirectX::XMMATRIX GetProj() const;

    DirectX::XMMATRIX GetViewProj() const;

    DirectX::XMFLOAT3 GetPosition() const { return mPosition; }

	DirectX::XMFLOAT4X4& GetProj4x4() { return mProj; }
    void SetViewMatrix(const DirectX::XMMATRIX& view);
    void SetPosition(const DirectX::XMFLOAT3& pos);

	D3D12_VIEWPORT GetViewport() const { return m_viewport; }
	D3D12_RECT GetScissorRect() const { return m_scissorRect; }

    void UpdateProjection(float fovY, float aspect, float zn, float zf);

    void ApplyToCommandList(ID3D12GraphicsCommandList* cmdList) const;

    DirectX::XMFLOAT4 GetRotationQuaternion() const;
private:
    D3D12_VIEWPORT m_viewport;
    D3D12_RECT m_scissorRect;

    DirectX::XMFLOAT4X4 mView = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 mProj = MathHelper::Identity4x4();

    DirectX::XMFLOAT3 mPosition = { 0.0f, 0.0f, 0.0f };

};