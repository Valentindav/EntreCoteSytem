#include "RenderCamera.h"
#include "Data/InstanceData.h"

using namespace DirectX;

RenderCamera::RenderCamera(int clientWidth, int clientHeight)
{
    int mClientWidth = clientWidth;
	int mClientHeight = clientHeight;
    SetViewport(mClientWidth, mClientHeight);
    m_scissorRect = { 0, 0, mClientWidth, mClientHeight };

    XMStoreFloat4x4(&mView, XMMatrixIdentity());
    XMStoreFloat4x4(&mProj, XMMatrixIdentity());
}

RenderCamera::~RenderCamera()
{
}

void RenderCamera::SetScissorRect(int left, int top, int right, int bottom)
{
    m_scissorRect.left = left;
    m_scissorRect.top = top;
    m_scissorRect.right = right;
    m_scissorRect.bottom = bottom;
}

void RenderCamera::ApplyViewport(ID3D12GraphicsCommandList* cmdList) const
{
    cmdList->RSSetViewports(1, &m_viewport);
    cmdList->RSSetScissorRects(1, &m_scissorRect);
}

void RenderCamera::SetViewport(float width, float height, float topLeftX, float topLeftY, float minDepth, float maxDepth)
{
    m_viewport.TopLeftX = topLeftX;
    m_viewport.TopLeftY = topLeftY;
    m_viewport.Width = width;
    m_viewport.Height = height;
    m_viewport.MinDepth = minDepth;
    m_viewport.MaxDepth = maxDepth;
}

void RenderCamera::SetLens(float fovY, float aspect, float nearZ, float farZ)
{
    XMMATRIX P = XMMatrixPerspectiveFovLH(fovY, aspect, nearZ, farZ);

    XMStoreFloat4x4(&mProj, P);
}

void RenderCamera::LookAt(const XMFLOAT3& pos, const XMFLOAT3& target, const XMFLOAT3& up)
{
    mPosition = pos;

    XMVECTOR P = XMLoadFloat3(&pos);
    XMVECTOR T = XMLoadFloat3(&target);
    XMVECTOR U = XMLoadFloat3(&up);

    XMMATRIX V = XMMatrixLookAtLH(P, T, U);
    XMStoreFloat4x4(&mView, V);
}

XMMATRIX RenderCamera::GetView() const
{
    return XMLoadFloat4x4(&mView);
}

XMMATRIX RenderCamera::GetProj() const
{
    return XMLoadFloat4x4(&mProj);
}

XMMATRIX RenderCamera::GetViewProj() const
{
    return XMLoadFloat4x4(&mView) * XMLoadFloat4x4(&mProj);
}

void RenderCamera::SetViewMatrix(const DirectX::XMMATRIX& view)
{
    DirectX::XMStoreFloat4x4(&mView, view);
}

void RenderCamera::SetPosition(const DirectX::XMFLOAT3& pos)
{
    mPosition = pos;
}

void RenderCamera::UpdateProjection(float fovY, float aspect, float zn, float zf) {
    XMMATRIX P = XMMatrixPerspectiveFovLH(fovY, aspect, zn, zf);
    XMStoreFloat4x4(&mProj, P);
}

void RenderCamera::ApplyToCommandList(ID3D12GraphicsCommandList* cmdList) const {
    cmdList->RSSetViewports(1, &m_viewport);
    cmdList->RSSetScissorRects(1, &m_scissorRect);
}

DirectX::XMFLOAT4 RenderCamera::GetRotationQuaternion() const
{

    DirectX::XMMATRIX view = DirectX::XMLoadFloat4x4(&mView);
    DirectX::XMMATRIX invView = DirectX::XMMatrixInverse(nullptr, view);

    DirectX::XMVECTOR quat = DirectX::XMQuaternionRotationMatrix(invView);

    DirectX::XMFLOAT4 result;
    DirectX::XMStoreFloat4(&result, quat);
    return result;
}