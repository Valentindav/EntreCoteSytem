#include "GpuDevice.h"
#include "SwapChain.h"
#include <iostream>

// Liens vers les librairies DirectX
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

GpuDevice::GpuDevice()
{
}

GpuDevice::~GpuDevice()
{
    Flush();
}

bool GpuDevice::Initialize()
{
    CreateDevice();

    CreateCommandObjects();

    CreateGlobalDescriptorHeap();

    return true;
}

void GpuDevice::CreateDevice()
{
#if defined(DEBUG) || defined(_DEBUG)
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();
        }
    }
#endif

ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&mdxgiFactory)));

// Essai de création du device Hardware
HRESULT hardwareResult = D3D12CreateDevice(
    nullptr,
    D3D_FEATURE_LEVEL_11_0,
    IID_PPV_ARGS(&md3dDevice)
);

// Fallback sur WARP (Software rasterizer) si pas de GPU compatible
if (FAILED(hardwareResult))
{
    ComPtr<IDXGIAdapter> pWarpAdapter;
    ThrowIfFailed(mdxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));
    ThrowIfFailed(D3D12CreateDevice(
        pWarpAdapter.Get(),
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&md3dDevice)
    ));
}

// Récupération des tailles de descripteurs pour l'alignement
mRtvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
mCbvSrvUavDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void GpuDevice::CreateCommandObjects()
{
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

    ThrowIfFailed(md3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue)));

    ThrowIfFailed(md3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));
}

void GpuDevice::CreateGlobalDescriptorHeap()
{
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    srvHeapDesc.NumDescriptors = MAX_DESCRIPTORS;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mGlobalDescriptorHeap)));
}

UINT GpuDevice::AllocateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* outCpuHandle)
{
    if (mCurrentDescriptorIndex >= MAX_DESCRIPTORS)
    {
        throw std::runtime_error("Out of descriptors in Global Heap!");
    }

    UINT index = mCurrentDescriptorIndex;
    mCurrentDescriptorIndex++;

    CD3DX12_CPU_DESCRIPTOR_HANDLE handle(
        mGlobalDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

    handle.Offset(index, mCbvSrvUavDescriptorSize);

    if (outCpuHandle) *outCpuHandle = handle;

    return index;
}

void GpuDevice::ExecuteCommandList(ID3D12CommandList* cmdList)
{
    ID3D12CommandList* cmdsLists[] = { cmdList };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
}

void GpuDevice::Flush()
{
    Signal();
    WaitForFence(mCurrentFence);
}

UINT64 GpuDevice::Signal()
{
    mCurrentFence++;
    ThrowIfFailed(mCommandQueue->Signal(mFence.Get(), mCurrentFence));
    return mCurrentFence;
}

void GpuDevice::WaitForFence(UINT64 fenceValue)
{
    if (mFence->GetCompletedValue() < fenceValue)
    {
        HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
        if (eventHandle == nullptr) return;

        ThrowIfFailed(mFence->SetEventOnCompletion(fenceValue, eventHandle));
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }
}

void GpuDevice::CreateSwapChain(HWND hwnd, int width, int height)
{
    mSwapChain = std::make_unique<SwapChain>(this, hwnd, width, height);
}

void GpuDevice::ResizeSwapChain(int width, int height)
{
    if (mSwapChain)
    {
        Flush();
        mSwapChain->Resize(width, height);
    }
}

void GpuDevice::Present()
{
    if (mSwapChain)
    {
        mSwapChain->Present();
    }
}