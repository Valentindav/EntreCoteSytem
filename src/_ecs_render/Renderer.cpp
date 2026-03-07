#include "Renderer.h"
#include "GpuDevice.h"
#include "GpuBufferConverter.h"
#include "GpuCommandDispatcher.h"
#include "RenderTarget.h"
#include "SwapChain.h"

#include <d3dcompiler.h>
#include <stdexcept>

#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;

// ============================================================
Renderer::Renderer() {}

Renderer::~Renderer()
{
    if (m_gpuCommandDispatcher) m_gpuCommandDispatcher->Flush();

    for (int i = 0; i < FrameCount; ++i)
    {
        if (m_instanceUploadBuffers[i]) m_instanceUploadBuffers[i]->Unmap(0, nullptr);
        if (m_passCB[i]) m_passCB[i]->Unmap(0, nullptr);
        if (m_lightsCB[i]) m_lightsCB[i]->Unmap(0, nullptr);
        if (m_boneUploadBuffers[i]) m_boneUploadBuffers[i]->Unmap(0, nullptr);
    }

    for (auto& pair : m_particleSystems)
    {
        if (pair.second) pair.second->Shutdown();
    }
    m_particleSystems.clear();
}

// ============================================================
//  Initialize 
// ============================================================
void Renderer::Initialize(HWND hwnd, int width, int height)
{
    m_gpuDevice = std::make_unique<GpuDevice>();
    if (!m_gpuDevice->Initialize()) throw std::runtime_error("Erreur init DX12");

    m_gpuCommandDispatcher = std::make_unique<GpuCommandDispatcher>();
    m_gpuCommandDispatcher->Initialize(m_gpuDevice->GetDevice(), m_gpuDevice->GetCommandQueue());
    m_gpuBufferConverter = std::make_unique<GpuBufferConverter>(m_gpuDevice.get());

    m_gpuDevice->CreateSwapChain(hwnd, width, height);
    m_renderTarget = std::make_unique<RenderTarget>();
    m_renderTarget->Initialize(m_gpuDevice->GetDevice(), m_gpuDevice->GetSwapChain(), width, height);

    BuildRootSignature();
    BuildUIRootSignature();
    BuildParticleRootSignatures();

    BuildShadersAndInputLayout();
    BuildUIShadersAndInputLayout();

    BuildPSO();
    BuildUIPSO();
    BuildParticlePSOs();

    BuildUIBuffers();

    // Allocations des buffers
    UINT instanceBufferSize = MaxInstancesPerFrame * sizeof(InstanceData);
    const int MAX_CAMERAS = 8;
    UINT passCBSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstants)) * MAX_CAMERAS;
    UINT lightCBSize = d3dUtil::CalcConstantBufferByteSize(sizeof(LightConstants));
    UINT boneBufferSize = MaxBonesPerFrame * sizeof(DirectX::XMFLOAT4X4);

    for (int i = 0; i < FrameCount; ++i)
    {
        auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

        // Instance Buffer
        auto instDesc = CD3DX12_RESOURCE_DESC::Buffer(instanceBufferSize);
        ThrowIfFailed(m_gpuDevice->GetDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &instDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_instanceUploadBuffers[i])));
        ThrowIfFailed(m_instanceUploadBuffers[i]->Map(0, nullptr, reinterpret_cast<void**>(&m_instanceBufferMapped[i])));

        // Pass CB
        auto passDesc = CD3DX12_RESOURCE_DESC::Buffer(passCBSize);
        ThrowIfFailed(m_gpuDevice->GetDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &passDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_passCB[i])));
        ThrowIfFailed(m_passCB[i]->Map(0, nullptr, reinterpret_cast<void**>(&m_passCBMapped[i])));

        // Light CB
        auto lightDesc = CD3DX12_RESOURCE_DESC::Buffer(lightCBSize);
        ThrowIfFailed(m_gpuDevice->GetDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &lightDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_lightsCB[i])));
        ThrowIfFailed(m_lightsCB[i]->Map(0, nullptr, reinterpret_cast<void**>(&m_lightsCBMapped[i])));

        // Bone Buffer
        auto boneDesc = CD3DX12_RESOURCE_DESC::Buffer(boneBufferSize);
        ThrowIfFailed(m_gpuDevice->GetDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &boneDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_boneUploadBuffers[i])));
        ThrowIfFailed(m_boneUploadBuffers[i]->Map(0, nullptr, reinterpret_cast<void**>(&m_boneBufferMapped[i])));
    }
}

void Renderer::SimulateParticles(const std::vector<ParticleRenderData>& activeParticles, float deltaTime)
{
    if (activeParticles.empty()) return;

    auto cmdList = m_gpuCommandDispatcher->GetCommandList();
    auto particleHeap = m_renderTarget->GetParticleHeap().Get();

    for (const ParticleRenderData& data : activeParticles)
    {
        if (!data.enabled) continue;

        // Vérifier/Créer le système GPU
        if (m_particleSystems.find(data.EmitterId) == m_particleSystems.end()) {
            UpdateEmitterGpu(data.EmitterId, data.maxParticles, cmdList);
        }

        auto& system = m_particleSystems[data.EmitterId];

        GPUParticleParams params = {};
        params.deltaTime = deltaTime;
        params.gravity = data.gravity;
        params.originX = data.originX; params.originY = data.originY; params.originZ = data.originZ;
        params.spawnRadius = data.spawnRadius;
        params.speed = data.speed;
        params.spread = data.spread;
        params.minLife = data.minLife; params.maxLife = data.maxLife;
        params.sizeStart = data.sizeStart; params.sizeEnd = data.sizeEnd;
        params.colorStartR = data.colorStartR; params.colorStartG = data.colorStartG;
        params.colorStartB = data.colorStartB; params.colorStartA = data.colorStartA;
        params.colorEndR = data.colorEndR; params.colorEndG = data.colorEndG;
        params.colorEndB = data.colorEndB; params.colorEndA = data.colorEndA;
        params.maxParticles = data.maxParticles;
        params.emitRate = data.emitRate;
        params.deltaAccum = data.deltaAccum;

        system->Dispatch(cmdList, m_particleComputeRootSig.Get(), m_psParticleCompute.Get(), particleHeap, params);
    }
}

// ============================================================
//  InitEmitterGpu
// ============================================================
bool Renderer::UpdateEmitterGpu(uint32_t emitterId, uint32_t maxParticles, ID3D12GraphicsCommandList* cmdList)
{
    auto heap = m_renderTarget->GetParticleHeap();
    UINT descSize = m_gpuDevice->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    UINT baseSlot = (UINT)m_particleSystems.size() * 2;

    auto system = std::make_unique<GPUParticleSystem>();
    bool success = system->Init(
        m_gpuDevice->GetDevice(),
        m_particleComputeRootSig.GetAddressOf(),
        m_psParticleCompute.GetAddressOf(),
        m_particleCS.Get(),
        heap->GetCPUDescriptorHandleForHeapStart(),
        heap->GetGPUDescriptorHandleForHeapStart(),
        descSize,
        baseSlot,
        maxParticles
    );

    if (success) {
        system->UploadInit(cmdList);

        m_particleSystems[emitterId] = std::move(system);
    }
    return success;
}

// ============================================================
//  OnResize
// ============================================================
void Renderer::OnResize(int width, int height)
{
    if (!m_gpuDevice) return;
    m_gpuCommandDispatcher->Flush();
    m_gpuDevice->ResizeSwapChain(width, height);
    m_renderTarget->Initialize(m_gpuDevice->GetDevice(), m_gpuDevice->GetSwapChain(), width, height);
}

// ============================================================
//  BeginFrame
// ============================================================
void Renderer::BeginFrame()
{
    auto cmdList = m_gpuCommandDispatcher->ResetCommandList();
    m_currFrameIndex = m_gpuCommandDispatcher->GetFrameIndex();
    m_instanceBufferOffset = 0;

    auto swapChain = m_gpuDevice->GetSwapChain();
    UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();
    auto backBuffer = swapChain->GetCurrentBackBuffer();

    auto rtvHandle = m_renderTarget->GetBackBufferView(backBufferIndex);
    auto dsvHandle = m_renderTarget->GetDepthStencilView();

    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    cmdList->ResourceBarrier(1, &barrier);

    float clearColor[] = { 0.0f, 0.994764f, 1.f, 1.0f };
    cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    cmdList->OMSetRenderTargets(1, &rtvHandle, TRUE, &dsvHandle);
    cmdList->SetGraphicsRootSignature(m_rootSignature.Get());
}

// ============================================================
//  BeginCamera
// ============================================================
void Renderer::BeginCamera(DirectX::XMMATRIX view, DirectX::XMMATRIX proj, DirectX::XMFLOAT3 position, const D3D12_VIEWPORT& viewport, const D3D12_RECT& scissorRect, int cameraIndex)
{
    auto cmdList = m_gpuCommandDispatcher->GetCommandList();
    cmdList->RSSetViewports(1, &viewport);
    cmdList->RSSetScissorRects(1, &scissorRect);

    cmdList->SetGraphicsRootSignature(m_rootSignature.Get());

    PassConstants passCB;
    XMStoreFloat4x4(&passCB.ViewProj, XMMatrixMultiply(view, proj));
    XMStoreFloat4x4(&passCB.View, view);
    passCB.EyePosW = position;
    passCB.TotalTime = 0.0f;

    UINT cbSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstants));
    memcpy(m_passCBMapped[m_currFrameIndex] + (cameraIndex * cbSize), &passCB, sizeof(PassConstants));

    D3D12_GPU_VIRTUAL_ADDRESS cbAddress = m_passCB[m_currFrameIndex]->GetGPUVirtualAddress() + (cameraIndex * cbSize);
    m_currentPassCBAddress = cbAddress;

    cmdList->SetGraphicsRootConstantBufferView(0, cbAddress);

    if (cameraIndex > 0)
    {
        auto dsvHandle = m_renderTarget->GetDepthStencilView();

        cmdList->ClearDepthStencilView(
            dsvHandle,
            D3D12_CLEAR_FLAG_DEPTH,
            1.0f,
            0,
            0,
            nullptr
        );

        auto swapChain = m_gpuDevice->GetSwapChain();
        UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();
        auto rtvHandle = m_renderTarget->GetBackBufferView(backBufferIndex);

        float minimapBgColor[] = { 0.0f, 0.0052356020f, 1.f, 1.0f };

        cmdList->ClearRenderTargetView(
            rtvHandle,
            minimapBgColor,
            1,
            &scissorRect
        );
    }
}

// ============================================================
//  RenderScene
// ============================================================
void Renderer::RenderScene(const std::unordered_map<MeshHandle, BatchData>& batches,
    const std::vector<LightData>& activeLights,
    const std::vector<DirectX::XMFLOAT4X4>& frameBoneTransforms,
    const std::vector<ParticleRenderData>& activeParticles,
    uint8_t filter)
{
    auto cmdList = m_gpuCommandDispatcher->GetCommandList();
    auto srvHeap = m_gpuDevice->GetSrvHeap();

    // 1. Mise à jour des Lumières (Slot 1)
    LightConstants lightCB;
    lightCB.LightCount = (int)min(activeLights.size(), (size_t)MAX_LIGHTS);
    for (int i = 0; i < lightCB.LightCount; ++i) lightCB.Lights[i] = activeLights[i];
    memcpy(m_lightsCBMapped[m_currFrameIndex], &lightCB, sizeof(LightConstants));
    cmdList->SetGraphicsRootConstantBufferView(1, m_lightsCB[m_currFrameIndex]->GetGPUVirtualAddress());

    // 2. Skinning & Animation
    BYTE* mappedBones = m_boneBufferMapped[m_currFrameIndex];
    if (!frameBoneTransforms.empty())
    {
        size_t copySize = min((UINT)frameBoneTransforms.size(), MaxBonesPerFrame);
        memcpy(mappedBones, frameBoneTransforms.data(), copySize * sizeof(DirectX::XMFLOAT4X4));
    }
    else
    {
        DirectX::XMFLOAT4X4 identity = MathHelper::Identity4x4();
        memcpy(mappedBones, &identity, sizeof(DirectX::XMFLOAT4X4));
    }

    cmdList->SetGraphicsRootShaderResourceView(3, m_boneUploadBuffers[m_currFrameIndex]->GetGPUVirtualAddress());

    // 3. Rendu des Objets Opaques
    ID3D12DescriptorHeap* heaps[] = { srvHeap };
    cmdList->SetDescriptorHeaps(_countof(heaps), heaps);
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    for (const auto& [meshHandle, batch] : batches)
    {
        if (batch.instances.empty()) continue;

        if (filter & RenderFlags::Wireframe) cmdList->SetPipelineState(m_psoWireframe.Get());
        else cmdList->SetPipelineState(m_psoSolid.Get());

        cmdList->SetGraphicsRootDescriptorTable(2, srvHeap->GetGPUDescriptorHandleForHeapStart());

        MeshBuffers mesh = m_gpuBufferConverter->GetMeshBuffers(meshHandle);
        cmdList->IASetVertexBuffers(0, 1, &mesh.VertexBufferView);
        cmdList->IASetIndexBuffer(&mesh.IndexBufferView);

        D3D12_GPU_VIRTUAL_ADDRESS instAddress = UploadInstanceData(batch.instances);

        D3D12_VERTEX_BUFFER_VIEW instVbv = {};
        instVbv.BufferLocation = instAddress;
        instVbv.SizeInBytes = (UINT)batch.instances.size() * sizeof(InstanceData);
        instVbv.StrideInBytes = sizeof(InstanceData);

        cmdList->IASetVertexBuffers(1, 1, &instVbv);
        cmdList->DrawIndexedInstanced(mesh.IndexCount, (UINT)batch.instances.size(), 0, 0, 0);
    }

    // 4. Rendu des Particules
    if (!activeParticles.empty())
    {
        auto particleHeap = m_renderTarget->GetParticleHeap().Get();

        for (const ParticleRenderData& data : activeParticles)
        {
            if (!data.enabled) continue;
            if (m_particleSystems.find(data.EmitterId) == m_particleSystems.end()) continue;

            auto& system = m_particleSystems[data.EmitterId];

            GPUParticleRenderCB rcb = {};
            rcb.sizeStart = data.sizeStart; rcb.sizeEnd = data.sizeEnd;
            rcb.colorStartR = data.colorStartR; rcb.colorStartG = data.colorStartG;
            rcb.colorStartB = data.colorStartB; rcb.colorStartA = data.colorStartA;
            rcb.colorEndR = data.colorEndR; rcb.colorEndG = data.colorEndG;
            rcb.colorEndB = data.colorEndB; rcb.colorEndA = data.colorEndA;

            cmdList->SetGraphicsRootSignature(m_particleRenderRootSig.Get());
            cmdList->SetGraphicsRootConstantBufferView(0, m_currentPassCBAddress);

            system->Draw(cmdList, m_particleRenderRootSig.Get(), m_psParticleRender.Get(), particleHeap, rcb);
        }
    }
}

// ============================================================
//  RenderUI
// ============================================================
void Renderer::RenderUI(const std::vector<UIVertex>& vertices, UploadBuffer<UIVertex>* uiVertexBuffer, float screenWidth, float screenHeight, const D3D12_VIEWPORT& viewport, const D3D12_RECT& scissorRect)
{
    if (vertices.empty() || !uiVertexBuffer) return;

    auto cmdList = m_gpuCommandDispatcher->GetCommandList();
    auto srvHeap = m_gpuDevice->GetSrvHeap();

    cmdList->RSSetViewports(1, &viewport);
    cmdList->RSSetScissorRects(1, &scissorRect);

    cmdList->SetPipelineState(uiPSO.Get());
    cmdList->SetGraphicsRootSignature(uiRootSignature.Get());

    ID3D12DescriptorHeap* heaps[] = { srvHeap };
    cmdList->SetDescriptorHeaps(_countof(heaps), heaps);
    cmdList->SetGraphicsRootDescriptorTable(1, srvHeap->GetGPUDescriptorHandleForHeapStart());

    UIConstants constants = {};
    constants.ScreenSize = { screenWidth, screenHeight };
    uiCB->CopyData(0, constants);

    cmdList->SetGraphicsRootConstantBufferView(0, uiCB->Resource()->GetGPUVirtualAddress());

    for (size_t i = 0; i < vertices.size(); ++i) {
        uiVertexBuffer->CopyData((int)i, vertices[i]);
    }

    D3D12_VERTEX_BUFFER_VIEW vbv;
    vbv.BufferLocation = uiVertexBuffer->Resource()->GetGPUVirtualAddress();
    vbv.StrideInBytes = sizeof(UIVertex);
    vbv.SizeInBytes = (UINT)vertices.size() * sizeof(UIVertex);

    cmdList->IASetVertexBuffers(0, 1, &vbv);
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmdList->DrawInstanced((UINT)vertices.size(), 1, 0, 0);
}

// ============================================================
//  EndFrame
// ============================================================
void Renderer::EndFrame()
{
    auto cmdList = m_gpuCommandDispatcher->GetCommandList();
    auto backBuffer = m_gpuDevice->GetSwapChain()->GetCurrentBackBuffer();

    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    cmdList->ResourceBarrier(1, &barrier);

    m_gpuCommandDispatcher->ExecuteCommandList();
    m_gpuDevice->Present();
    m_gpuCommandDispatcher->Flush();
}

// ============================================================
//  UploadInstanceData
// ============================================================
D3D12_GPU_VIRTUAL_ADDRESS Renderer::UploadInstanceData(const std::vector<InstanceData>& instances)
{
    UINT numInstances = (UINT)instances.size();
    UINT dataSize = numInstances * sizeof(InstanceData);

    if (m_instanceBufferOffset + dataSize > MaxInstancesPerFrame * sizeof(InstanceData)) {
        throw std::runtime_error("Renderer::UploadInstanceData - Buffer plein !");
    }

    memcpy(m_instanceBufferMapped[m_currFrameIndex] + m_instanceBufferOffset, instances.data(), dataSize);

    D3D12_GPU_VIRTUAL_ADDRESS address = m_instanceUploadBuffers[m_currFrameIndex]->GetGPUVirtualAddress() + m_instanceBufferOffset;
    m_instanceBufferOffset += dataSize;

    return address;
}

// -----------------------------------------------------------
// INITIALISATION DES PIPELINES & SHADERS
// -----------------------------------------------------------

// ============================================================
//  BuildRootSignature (scene)
// ============================================================
void Renderer::BuildRootSignature()
{
    CD3DX12_DESCRIPTOR_RANGE srvTable;
    srvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 100, 0);

    CD3DX12_ROOT_PARAMETER slotRootParameter[4];
    slotRootParameter[0].InitAsConstantBufferView(0); // b0 : Pass
    slotRootParameter[1].InitAsConstantBufferView(1); // b1 : Lights
    slotRootParameter[2].InitAsDescriptorTable(1, &srvTable, D3D12_SHADER_VISIBILITY_PIXEL); // t0-t99
    slotRootParameter[3].InitAsShaderResourceView(100); // t100 : Bones Structured Buffer

    CD3DX12_STATIC_SAMPLER_DESC sampler(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR,
        D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        D3D12_TEXTURE_ADDRESS_MODE_WRAP);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter, 1, &sampler,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    ThrowIfFailed(D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serializedRootSig, &errorBlob));
    if (errorBlob) ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    ThrowIfFailed(m_gpuDevice->GetDevice()->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
}

// ============================================================
//  BuildUIRootSignature
// ============================================================
void Renderer::BuildUIRootSignature()
{
    CD3DX12_DESCRIPTOR_RANGE texTable;
    texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1000, 0); // t0

    CD3DX12_ROOT_PARAMETER slotRootParameter[2];
    slotRootParameter[0].InitAsConstantBufferView(0); // b0
    slotRootParameter[1].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);

    CD3DX12_STATIC_SAMPLER_DESC sampler(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR); // s0

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
        2, slotRootParameter, 1, &sampler,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;

    ThrowIfFailed(D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf()));

    ThrowIfFailed(m_gpuDevice->GetDevice()->CreateRootSignature(
        0,
        serializedRootSig->GetBufferPointer(),
        serializedRootSig->GetBufferSize(),
        IID_PPV_ARGS(&uiRootSignature)));
}

// ============================================================
//  BuildParticleRootSignatures
// ============================================================
void Renderer::BuildParticleRootSignatures()
{
    ID3D12Device* dev = m_gpuDevice->GetDevice();

    // -- Compute : b0=CB, u0=UAV
    {
        D3D12_DESCRIPTOR_RANGE uavRange = {};
        uavRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
        uavRange.NumDescriptors = 1;
        uavRange.BaseShaderRegister = 0;

        D3D12_ROOT_PARAMETER p[2] = {};
        p[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        p[0].Descriptor.ShaderRegister = 0;
        p[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        p[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        p[1].DescriptorTable.NumDescriptorRanges = 1;
        p[1].DescriptorTable.pDescriptorRanges = &uavRange;
        p[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        D3D12_ROOT_SIGNATURE_DESC desc = {};
        desc.NumParameters = 2;
        desc.pParameters = p;

        ComPtr<ID3DBlob> blob, err;
        ThrowIfFailed(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1,
            blob.GetAddressOf(), err.GetAddressOf()));
        ThrowIfFailed(dev->CreateRootSignature(0, blob->GetBufferPointer(),
            blob->GetBufferSize(), IID_PPV_ARGS(m_particleComputeRootSig.GetAddressOf())));
    }

    // -- Render : b0=PassCB  b1=ParticleRenderCB  t0=SRV
    {
        D3D12_DESCRIPTOR_RANGE srvRange = {};
        srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        srvRange.NumDescriptors = 1;
        srvRange.BaseShaderRegister = 0;

        D3D12_ROOT_PARAMETER p[3] = {};
        p[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        p[0].Descriptor.ShaderRegister = 0;
        p[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        p[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        p[1].Descriptor.ShaderRegister = 1;
        p[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        p[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        p[2].DescriptorTable.NumDescriptorRanges = 1;
        p[2].DescriptorTable.pDescriptorRanges = &srvRange;
        p[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

        D3D12_ROOT_SIGNATURE_DESC desc = {};
        desc.NumParameters = 3;
        desc.pParameters = p;
        desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        ComPtr<ID3DBlob> blob, err;
        ThrowIfFailed(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1,
            blob.GetAddressOf(), err.GetAddressOf()));
        ThrowIfFailed(dev->CreateRootSignature(0, blob->GetBufferPointer(),
            blob->GetBufferSize(), IID_PPV_ARGS(m_particleRenderRootSig.GetAddressOf())));
    }
}

// ============================================================
//  BuildShadersAndInputLayout
// ============================================================
void Renderer::BuildShadersAndInputLayout()
{
    m_vsByteCode = d3dUtil::CompileShader(L"../../res/Shaders/color.hlsl", nullptr, "VS", "vs_5_1");
    m_psSolidByteCode = d3dUtil::CompileShader(L"../../res/Shaders/color.hlsl", nullptr, "PS_Solid", "ps_5_1");
    m_psWireframeByteCode = d3dUtil::CompileShader(L"../../res/Shaders/color.hlsl", nullptr, "PS_Wireframe", "ps_5_1");

    m_inputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDWEIGHT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT,   0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0,  D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
        { "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
        { "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
        { "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
        { "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
        { "TEXINDEX", 0, DXGI_FORMAT_R32_UINT,          1, 80, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
        { "MATINDEX", 0, DXGI_FORMAT_R32_UINT,          1, 84, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
        { "BONEOFFSET", 0, DXGI_FORMAT_R32_UINT,        1, 88, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 }
    };

    if (!m_vsByteCode || !m_psSolidByteCode) {
        throw std::runtime_error("Erreur : Shader non charge. Verifiez le chemin du fichier .hlsl !");
    }
}

// ============================================================
//  BuildUIShadersAndInputLayout
// ============================================================
void Renderer::BuildUIShadersAndInputLayout()
{
    uiVS = d3dUtil::CompileShader(L"../../res/Shaders/ui.hlsl", nullptr, "VS", "vs_5_1");
    uiPS = d3dUtil::CompileShader(L"../../res/Shaders/ui.hlsl", nullptr, "PS", "ps_5_1");

    uiInputLayout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 8,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 1, DXGI_FORMAT_R32_FLOAT,          0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
}

// ============================================================
//  BuildPSO (scene)
// ============================================================
void Renderer::BuildPSO()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { m_inputLayout.data(), (UINT)m_inputLayout.size() };
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS = { reinterpret_cast<BYTE*>(m_vsByteCode->GetBufferPointer()),      m_vsByteCode->GetBufferSize() };
    psoDesc.PS = { reinterpret_cast<BYTE*>(m_psSolidByteCode->GetBufferPointer()), m_psSolidByteCode->GetBufferSize() };
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleDesc.Quality = 0;

    ThrowIfFailed(m_gpuDevice->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_psoSolid)));

    CD3DX12_RASTERIZER_DESC wireframeRasterizer = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    wireframeRasterizer.FillMode = D3D12_FILL_MODE_WIREFRAME;
    wireframeRasterizer.CullMode = D3D12_CULL_MODE_NONE;
    wireframeRasterizer.DepthBias = -50;
    wireframeRasterizer.SlopeScaledDepthBias = -1.0f;
    wireframeRasterizer.DepthBiasClamp = 0.0f;

    psoDesc.RasterizerState = wireframeRasterizer;
    psoDesc.PS.pShaderBytecode = reinterpret_cast<BYTE*>(m_psWireframeByteCode->GetBufferPointer());
    psoDesc.PS.BytecodeLength = m_psWireframeByteCode->GetBufferSize();

    ThrowIfFailed(m_gpuDevice->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_psoWireframe)));
}

// ============================================================
//  BuildParticlePSOs
// ============================================================
void Renderer::BuildParticlePSOs()
{
    m_particleCS = d3dUtil::CompileShader(L"../../res/Shaders/SimpleParticle.hlsl", nullptr, "main", "cs_5_1");
    if (!m_particleCS) throw std::runtime_error("SimpleParticle.hlsl CS failed");

    m_particleVS = d3dUtil::CompileShader(L"../../res/Shaders/ParticleRender.hlsl", nullptr, "VS", "vs_5_1");
    if (!m_particleVS) throw std::runtime_error("ParticleRender.hlsl VS failed");

    m_particlePS = d3dUtil::CompileShader(L"../../res/Shaders/ParticleRender.hlsl", nullptr, "PS", "ps_5_1");
    if (!m_particlePS) throw std::runtime_error("ParticleRender.hlsl PS failed");

    // Compute PSO
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC pso = {};
        pso.pRootSignature = m_particleComputeRootSig.Get();
        pso.CS.pShaderBytecode = m_particleCS->GetBufferPointer();
        pso.CS.BytecodeLength = m_particleCS->GetBufferSize();
        ThrowIfFailed(m_gpuDevice->GetDevice()->CreateComputePipelineState(
            &pso, IID_PPV_ARGS(m_psParticleCompute.GetAddressOf())));
    }

    // Render PSO
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC pso = {};
        pso.pRootSignature = m_particleRenderRootSig.Get();
        pso.VS = { m_particleVS->GetBufferPointer(), m_particleVS->GetBufferSize() };
        pso.PS = { m_particlePS->GetBufferPointer(), m_particlePS->GetBufferSize() };
        pso.InputLayout = { nullptr, 0 };

        pso.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        pso.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

        D3D12_BLEND_DESC blend = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        auto& rt = blend.RenderTarget[0];
        rt.BlendEnable = TRUE;
        rt.SrcBlend = D3D12_BLEND_SRC_ALPHA;
        rt.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
        rt.BlendOp = D3D12_BLEND_OP_ADD;
        rt.SrcBlendAlpha = D3D12_BLEND_ONE;
        rt.DestBlendAlpha = D3D12_BLEND_ZERO;
        rt.BlendOpAlpha = D3D12_BLEND_OP_ADD;
        pso.BlendState = blend;

        D3D12_DEPTH_STENCIL_DESC ds = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        ds.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
        pso.DepthStencilState = ds;

        pso.SampleMask = UINT_MAX;
        pso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        pso.NumRenderTargets = 1;
        pso.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        pso.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
        pso.SampleDesc.Count = 1;

        ThrowIfFailed(m_gpuDevice->GetDevice()->CreateGraphicsPipelineState(
            &pso, IID_PPV_ARGS(m_psParticleRender.GetAddressOf())));
    }
}

// ============================================================
//  BuildUIPSO
// ============================================================
void Renderer::BuildUIPSO()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { uiInputLayout.data(), (UINT)uiInputLayout.size() };
    psoDesc.pRootSignature = uiRootSignature.Get();
    psoDesc.VS = { uiVS->GetBufferPointer(), uiVS->GetBufferSize() };
    psoDesc.PS = { uiPS->GetBufferPointer(), uiPS->GetBufferSize() };

    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

    D3D12_BLEND_DESC blendDesc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    auto& rt = blendDesc.RenderTarget[0];
    rt.BlendEnable = TRUE; rt.SrcBlend = D3D12_BLEND_SRC_ALPHA; rt.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    rt.BlendOp = D3D12_BLEND_OP_ADD; rt.SrcBlendAlpha = D3D12_BLEND_ONE;
    rt.DestBlendAlpha = D3D12_BLEND_ZERO; rt.BlendOpAlpha = D3D12_BLEND_OP_ADD;
    psoDesc.BlendState = blendDesc;

    D3D12_DEPTH_STENCIL_DESC depthDesc = {};
    depthDesc.DepthEnable = FALSE;
    depthDesc.StencilEnable = FALSE;
    psoDesc.DepthStencilState = depthDesc;

    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;

    ThrowIfFailed(m_gpuDevice->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&uiPSO)));
}

// ============================================================
//  BuildUIBuffers
// ============================================================
void Renderer::BuildUIBuffers()
{
    uiCB = std::make_unique<UploadBuffer<UIConstants>>(m_gpuDevice->GetDevice(), 1, true);
}