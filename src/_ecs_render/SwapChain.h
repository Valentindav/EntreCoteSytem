#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <vector>
#include "d3dUtil.h"

using Microsoft::WRL::ComPtr;

class GpuDevice;

class SwapChain
{
public:
    SwapChain(GpuDevice* device, HWND hwnd, int width, int height);
    ~SwapChain();

    SwapChain(const SwapChain&) = delete;
    SwapChain& operator=(const SwapChain&) = delete;

    void Resize(int width, int height);

    void Present();

    UINT GetCurrentBackBufferIndex() const { return mDXGISwapChain->GetCurrentBackBufferIndex(); }

    ID3D12Resource* GetBackBuffer(UINT index) const
    {
        if (index >= BufferCount) return nullptr;
        return mBackBuffers[index].Get();
    }

    ID3D12Resource* GetCurrentBackBuffer() const
    {
        return mBackBuffers[mDXGISwapChain->GetCurrentBackBufferIndex()].Get();
    }

    D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferView() const;

    DXGI_FORMAT GetFormat() const { return mBackBufferFormat; }

    static const int BufferCount = 2;

private:
    void CreateSwapChain();
    void CreateRtvHeap();
    void CreateRenderTargetViews();

private:
    GpuDevice* mDevice = nullptr;
    HWND mHwnd = nullptr;

    int mWidth = 0;
    int mHeight = 0;
    DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

    ComPtr<IDXGISwapChain3> mDXGISwapChain;
    ComPtr<ID3D12Resource> mBackBuffers[BufferCount];

    ComPtr<ID3D12DescriptorHeap> mRtvHeap;
    UINT mRtvDescriptorSize = 0;

    UINT mCurrentBackBufferIndex = 0;
};