#include "GpuCommandDispatcher.h"

GpuCommandDispatcher::GpuCommandDispatcher()
{
}

GpuCommandDispatcher::~GpuCommandDispatcher()
{
    Flush();
    if (m_fenceEvent)
    {
        CloseHandle(m_fenceEvent);
    }
}

void GpuCommandDispatcher::Initialize(ID3D12Device* device, ID3D12CommandQueue* queue)
{
    m_device = device;
    m_commandQueue = queue;

    for (int i = 0; i < FrameCount; ++i)
    {
        ThrowIfFailed(m_device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(&m_commandAllocators[i])));

        m_fenceValues[i] = 0;
    }

    ThrowIfFailed(m_device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        m_commandAllocators[0].Get(),
        nullptr,
        IID_PPV_ARGS(&m_commandList)));

    m_commandList->Close();

    ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

ID3D12GraphicsCommandList* GpuCommandDispatcher::ResetCommandList()
{
    UINT64 fenceToWaitFor = m_fenceValues[m_frameIndex];

    if (m_fence->GetCompletedValue() < fenceToWaitFor)
    {
        ThrowIfFailed(m_fence->SetEventOnCompletion(fenceToWaitFor, m_fenceEvent));
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }

    auto allocator = m_commandAllocators[m_frameIndex].Get();
    ThrowIfFailed(allocator->Reset());

    ThrowIfFailed(m_commandList->Reset(allocator, nullptr));

    return m_commandList.Get();
}

UINT64 GpuCommandDispatcher::ExecuteCommandList(bool advanceFrameIndex)
{
    ThrowIfFailed(m_commandList->Close());

    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    m_currentFenceValue++;

    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_currentFenceValue));

    m_fenceValues[m_frameIndex] = m_currentFenceValue;

    if (advanceFrameIndex) {
        m_frameIndex = (m_frameIndex + 1) % FrameCount;
    }

    return m_currentFenceValue;
}

void GpuCommandDispatcher::Flush()
{
    m_currentFenceValue++;
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_currentFenceValue));

    if (m_fence->GetCompletedValue() < m_currentFenceValue)
    {
        ThrowIfFailed(m_fence->SetEventOnCompletion(m_currentFenceValue, m_fenceEvent));
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }
}