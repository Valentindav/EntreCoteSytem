#pragma once

#include "d3dUtil.h"
#include <wrl.h>
#include <d3d12.h>
#include <memory>
using namespace DirectX;
using Microsoft::WRL::ComPtr;
class SwapChain;

class RenderTarget
{
public:
    RenderTarget();
    ~RenderTarget();

    void Initialize(ID3D12Device* device, SwapChain* swapChain, int width, int height);
    void InitUiHeap(ID3D12Device* device);

    void OnResize(int width, int height, SwapChain* swapChain);

    D3D12_CPU_DESCRIPTOR_HANDLE GetBackBufferView(UINT bufferIndex) const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const;

    DXGI_FORMAT GetDepthStencilFormat() const { return m_depthStencilFormat; }

    ID3D12Resource* GetDepthStencilResource() const { return m_depthStencilBuffer.Get(); }

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetUiSrvHeap(){
        return uiSrvHeap;
    }

    void CreateUISRV(ID3D12Device* device, ComPtr<ID3D12Resource> texture, ID3D12DescriptorHeap* heap, int index);
    // Ajoute juste ces deux membres et la méthode

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetParticleHeap() { return m_particleHeap; }

	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }

private:
    void CreateDescriptorHeaps();
    void CreateRTVs(SwapChain* swapChain);
    void CreateDepthStencilBuffer(int width, int height);


private:
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_particleHeap = nullptr;


    ID3D12Device* m_device = nullptr;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvHeap;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> uiSrvHeap = nullptr;

    UINT m_rtvDescriptorSize = 0;
    UINT m_dsvDescriptorSize = 0;

    Microsoft::WRL::ComPtr<ID3D12Resource> m_depthStencilBuffer;

    DXGI_FORMAT m_depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    int m_width = 0;
    int m_height = 0;
};