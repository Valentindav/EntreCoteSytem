#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <vector>
#include <string>
#include <queue>
#include <mutex>
#include <memory>
#include <stdexcept>

#include "d3dUtil.h"

using Microsoft::WRL::ComPtr;

class SwapChain;

class GpuDevice
{
public:
    GpuDevice();
    ~GpuDevice();

    GpuDevice(const GpuDevice&) = delete;
    GpuDevice& operator=(const GpuDevice&) = delete;

    bool Initialize();

    // Soumission des commandes
    void ExecuteCommandList(ID3D12CommandList* cmdList);
    void Flush();
    UINT64 Signal();
    void WaitForFence(UINT64 fenceValue);

    // Getters D3D12
    ID3D12Device* GetDevice() const { return md3dDevice.Get(); }
    ID3D12CommandQueue* GetCommandQueue() const { return mCommandQueue.Get(); }
    IDXGIFactory4* GetFactory() const { return mdxgiFactory.Get(); }

    // Gestion des Descripteurs
    ID3D12DescriptorHeap* GetGlobalDescriptorHeap() const { return mGlobalDescriptorHeap.Get(); }
    UINT GetDescriptorIncrementSize() const { return mRtvDescriptorSize; }
    UINT GetCbvSrvUavDescriptorIncrementSize() const { return mCbvSrvUavDescriptorSize; }

    // Allocateur linéaire simple (Reset à chaque frame idéalement, ou RingBuffer)
    UINT AllocateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* outCpuHandle);

    // Gestion de la SwapChain
    void CreateSwapChain(HWND hwnd, int width, int height);
    void ResizeSwapChain(int width, int height);
    void Present();

    SwapChain* GetSwapChain() const { return mSwapChain.get(); }
    ID3D12DescriptorHeap* GetSrvHeap() const { return mGlobalDescriptorHeap.Get(); }

private:
    void CreateDevice();
    void CreateCommandObjects();
    void CreateGlobalDescriptorHeap();

private:
    ComPtr<IDXGIFactory4> mdxgiFactory;
    ComPtr<ID3D12Device> md3dDevice;

    ComPtr<ID3D12CommandQueue> mCommandQueue;   

    // Synchronisation CPU/GPU
    ComPtr<ID3D12Fence> mFence;
    UINT64 mCurrentFence = 0;

    // Heap global (Shader Visible)
    static const UINT MAX_DESCRIPTORS = 10000;
    ComPtr<ID3D12DescriptorHeap> mGlobalDescriptorHeap;
    UINT mCbvSrvUavDescriptorSize = 0;
    UINT mRtvDescriptorSize = 0;
    UINT mCurrentDescriptorIndex = 0;

    // Pointeur unique vers la SwapChain
    std::unique_ptr<SwapChain> mSwapChain;
};