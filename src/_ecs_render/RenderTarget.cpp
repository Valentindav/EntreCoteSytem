#include "RenderTarget.h"
#include "SwapChain.h"

using Microsoft::WRL::ComPtr;

RenderTarget::RenderTarget()
{
}

RenderTarget::~RenderTarget()
{
}

void RenderTarget::Initialize(ID3D12Device* device, SwapChain* swapChain, int width, int height)
{
    m_device = device;
    m_width = width;
    m_height = height;

    CreateDescriptorHeaps();
    CreateRTVs(swapChain);
    CreateDepthStencilBuffer(width, height);
}

void RenderTarget::InitUiHeap(ID3D12Device* device)
{
}

void RenderTarget::OnResize(int width, int height, SwapChain* swapChain)
{
    m_width = width;
    m_height = height;

    if (m_width <= 0 || m_height <= 0) return;

    CreateRTVs(swapChain);

    CreateDepthStencilBuffer(width, height);
}

void RenderTarget::CreateDescriptorHeaps()
{
    // --- RTV Heap (Couleur) ---
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = SwapChain::BufferCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;
    ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

    m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // --- DSV Heap (Profondeur) ---
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask = 0;
    ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));

    m_dsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);


    if (!uiSrvHeap)
    {
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
        heapDesc.NumDescriptors = 1024;
        heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&uiSrvHeap)));
    }

    // Après la création de uiSrvHeap, ajoute :
    if (!m_particleHeap) {
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
        heapDesc.NumDescriptors = 64;
        heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_particleHeap)));
    }


}

void RenderTarget::CreateRTVs(SwapChain* swapChain)
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

    for (int i = 0; i < SwapChain::BufferCount; ++i)
    {
        ID3D12Resource* backBuffer = swapChain->GetBackBuffer(i);

        m_device->CreateRenderTargetView(backBuffer, nullptr, rtvHeapHandle);

        rtvHeapHandle.Offset(1, m_rtvDescriptorSize);
    }
}

void RenderTarget::CreateDepthStencilBuffer(int width, int height)
{
    if (width <= 0 || height <= 0) return;

    m_depthStencilBuffer.Reset();

    // Description de la texture de profondeur
    D3D12_RESOURCE_DESC depthStencilDesc;
    depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthStencilDesc.Alignment = 0;
    depthStencilDesc.Width = width;
    depthStencilDesc.Height = height;
    depthStencilDesc.DepthOrArraySize = 1;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE optClear;
    optClear.Format = m_depthStencilFormat;
    optClear.DepthStencil.Depth = 1.0f;
    optClear.DepthStencil.Stencil = 0;

    auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    ThrowIfFailed(m_device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &depthStencilDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &optClear,
        IID_PPV_ARGS(&m_depthStencilBuffer)
    ));

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format = m_depthStencilFormat;
    dsvDesc.Texture2D.MipSlice = 0;

    m_device->CreateDepthStencilView(
        m_depthStencilBuffer.Get(),
        &dsvDesc,
        m_dsvHeap->GetCPUDescriptorHandleForHeapStart()
    );
}

void RenderTarget::CreateUISRV(
    ID3D12Device* device,
    ComPtr<ID3D12Resource> texture,
    ID3D12DescriptorHeap* heap,
    int index)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = texture->GetDesc().Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = texture->GetDesc().MipLevels;

    CD3DX12_CPU_DESCRIPTOR_HANDLE handle(heap->GetCPUDescriptorHandleForHeapStart());
    UINT increment = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    handle.Offset(index, increment);

    device->CreateShaderResourceView(texture.Get(), &srvDesc, handle);
}

D3D12_CPU_DESCRIPTOR_HANDLE RenderTarget::GetBackBufferView(UINT bufferIndex) const
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(
        m_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
        bufferIndex,
        m_rtvDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE RenderTarget::GetDepthStencilView() const
{
    return m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
}