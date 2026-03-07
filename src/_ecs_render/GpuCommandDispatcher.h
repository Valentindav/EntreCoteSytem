#pragma once

#include "d3dUtil.h"
#include <vector>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

class GpuCommandDispatcher
{
public:
    GpuCommandDispatcher();
    ~GpuCommandDispatcher();

    void Initialize(ID3D12Device* device, ID3D12CommandQueue* queue);

    ID3D12GraphicsCommandList* ResetCommandList();

    UINT64 ExecuteCommandList(bool advanceFrameIndex = true);

    void Flush();

    ID3D12CommandQueue* GetCommandQueue() const { return m_commandQueue.Get(); }
    ID3D12GraphicsCommandList* GetCommandList() const { return m_commandList.Get(); }
    UINT GetFrameIndex() const { return m_frameIndex; }

private:

    static const int FrameCount = 2;

    ID3D12Device* m_device = nullptr;

    ComPtr<ID3D12CommandQueue> m_commandQueue;

    ComPtr<ID3D12GraphicsCommandList> m_commandList;

    ComPtr<ID3D12CommandAllocator> m_commandAllocators[FrameCount];

    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValues[FrameCount];
    UINT64 m_currentFenceValue = 0;
    HANDLE m_fenceEvent;

    UINT m_frameIndex = 0;
};