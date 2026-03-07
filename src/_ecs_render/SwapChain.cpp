#include "SwapChain.h"
#include "GpuDevice.h"
#include <iostream>

SwapChain::SwapChain(GpuDevice* device, HWND hwnd, int width, int height)
    : mDevice(device)
    , mHwnd(hwnd)
    , mWidth(width)
    , mHeight(height)
{
    assert(device);
    assert(hwnd);

    CreateSwapChain();
    CreateRtvHeap();
    CreateRenderTargetViews();
}

SwapChain::~SwapChain()
{
}

void SwapChain::CreateSwapChain()
{
    mDXGISwapChain.Reset();

    DXGI_SWAP_CHAIN_DESC1 sd = {};
    sd.Width = mWidth;
    sd.Height = mHeight;
    sd.Format = mBackBufferFormat;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = BufferCount;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    ComPtr<IDXGISwapChain1> swapChain1;
    ThrowIfFailed(mDevice->GetFactory()->CreateSwapChainForHwnd(
        mDevice->GetCommandQueue(),
        mHwnd,
        &sd,
        nullptr,
        nullptr,
        &swapChain1
    ));

    ThrowIfFailed(swapChain1.As(&mDXGISwapChain));
    mCurrentBackBufferIndex = mDXGISwapChain->GetCurrentBackBufferIndex();
}

void SwapChain::CreateRtvHeap()
{
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = BufferCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;

    ThrowIfFailed(mDevice->GetDevice()->CreateDescriptorHeap(
        &rtvHeapDesc, IID_PPV_ARGS(&mRtvHeap)));

    mRtvDescriptorSize = mDevice->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
}

void SwapChain::CreateRenderTargetViews()
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());

    for (UINT i = 0; i < BufferCount; i++)
    {
        ThrowIfFailed(mDXGISwapChain->GetBuffer(i, IID_PPV_ARGS(&mBackBuffers[i])));

        mDevice->GetDevice()->CreateRenderTargetView(
            mBackBuffers[i].Get(),
            nullptr,
            rtvHeapHandle
        );

        rtvHeapHandle.Offset(1, mRtvDescriptorSize);
    }
}

void SwapChain::Resize(int width, int height)
{
    if (mWidth == width && mHeight == height) return;

    mWidth = width;
    mHeight = height;

    mDevice->Flush();

    for (int i = 0; i < BufferCount; ++i)
    {
        mBackBuffers[i].Reset();
    }

    DXGI_SWAP_CHAIN_DESC desc = {};
    mDXGISwapChain->GetDesc(&desc);
    ThrowIfFailed(mDXGISwapChain->ResizeBuffers(
        BufferCount,
        mWidth,
        mHeight,
        mBackBufferFormat,
        desc.Flags
    ));

    CreateRenderTargetViews();
}

D3D12_CPU_DESCRIPTOR_HANDLE SwapChain::GetCurrentBackBufferView() const
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(
        mRtvHeap->GetCPUDescriptorHandleForHeapStart(),
        mDXGISwapChain->GetCurrentBackBufferIndex(),
        mRtvDescriptorSize
    );
}

void SwapChain::Present()
{
    HRESULT hr = mDXGISwapChain->Present(1, 0);
    if (FAILED(hr))
    {
        HRESULT reason = mDevice->GetDevice()->GetDeviceRemovedReason();
        std::cout << "DEVICE REMOVED ! Code: 0x" << std::hex << reason << std::endl;

        // Codes courants :
        // 0x887a0006 = HUNG (Shader trop long ou boucle infinie)
        // 0x887a0005 = INVALID_CALL (Commande illégale envoyée)
        // 0x887a002b = OUT_OF_MEMORY

        throw std::runtime_error("Device Removed");
    }
    mCurrentBackBufferIndex = mDXGISwapChain->GetCurrentBackBufferIndex();
}