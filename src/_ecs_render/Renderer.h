#pragma once

#include "d3dUtil.h"

#include "UploadBuffer.h"
#include "GpuTypes.h"
#include "GpuDevice.h"
#include "GpuParticleSystem.h"

#include "Data/InstanceData.h"
#include "Data/LightData.h"
#include "Data/BatchData.h"
#include "Data/UiData.h"
#include "Data/ParticleRenderData.h"

#include <vector>
#include <unordered_map>
#include <memory>
#include <d3d12.h>
#include <wrl.h>
#include <mutex>

using Microsoft::WRL::ComPtr;

class GpuBufferConverter;
class GpuCommandDispatcher;
class RenderTarget;

struct PassConstants
{
    DirectX::XMFLOAT4X4 ViewProj = MathHelper::Identity4x4();  // 64 bytes
    DirectX::XMFLOAT4X4 View = MathHelper::Identity4x4();  // 64 bytes
    DirectX::XMFLOAT3   EyePosW = { 0.0f, 0.0f, 0.0f };
    float               TotalTime = 0.0f;
};

#define MAX_LIGHTS 16

struct LightConstants
{
    LightData Lights[MAX_LIGHTS];
    int LightCount = 0;
    float Pad[63];
};

class Renderer
{
public:
    Renderer();
    ~Renderer();

    void Initialize(HWND hwnd, int width, int height);
    void OnResize(int width, int height);

    void BeginFrame();

    void SimulateParticles(const std::vector<ParticleRenderData>& activeParticles, float deltaTime);

    void BeginCamera(DirectX::XMMATRIX view,
        DirectX::XMMATRIX proj,
        DirectX::XMFLOAT3 position,
        const D3D12_VIEWPORT& viewport,
        const D3D12_RECT& scissorRect,
        int cameraIndex);

    void RenderScene(
        const std::unordered_map<MeshHandle, BatchData>& batches,
        const std::vector<LightData>& activeLights,
        const std::vector<DirectX::XMFLOAT4X4>& boneTransforms,
        const std::vector<ParticleRenderData>& activeParticles,
        uint8_t filter
    );

    void RenderUI(
        const std::vector<UIVertex>& vertices,
        UploadBuffer<UIVertex>* uiVertexBuffer,
        float screenWidth, float screenHeight,
        const D3D12_VIEWPORT& viewport,
        const D3D12_RECT& scissorRect
    );

    void EndFrame();

    GpuBufferConverter* GetBufferConverter() const { return m_gpuBufferConverter.get(); }
    GpuCommandDispatcher* GetCommandDispatcher() const { return m_gpuCommandDispatcher.get(); }
    RenderTarget* GetRenderTarget() const { return m_renderTarget.get(); }
    ID3D12Device* GetDevice() const { return m_gpuDevice->GetDevice(); }
    //std::mutex& GetGraphicsMutex() { return m_graphicMutex; }

private:
    void BuildRootSignature();
    void BuildUIRootSignature();
    void BuildParticleRootSignatures();

    void BuildShadersAndInputLayout();
    void BuildUIShadersAndInputLayout();

    void BuildPSO();
    void BuildUIPSO();
    void BuildParticlePSOs();

    void BuildUIBuffers();
    D3D12_GPU_VIRTUAL_ADDRESS UploadInstanceData(const std::vector<InstanceData>& instances);

    bool UpdateEmitterGpu(uint32_t emitterId, uint32_t maxParticles, ID3D12GraphicsCommandList* cmdList);

private:
    static const int FrameCount = 2;
    static const UINT MaxInstancesPerFrame = 1000000;
    static const UINT MaxBonesPerFrame = 100000;

    // --- LES SOUS-SYSTÈMES DU RENDERER ---
    std::unique_ptr<GpuDevice> m_gpuDevice;
    std::unique_ptr<GpuBufferConverter> m_gpuBufferConverter;
    std::unique_ptr<GpuCommandDispatcher> m_gpuCommandDispatcher;
    std::unique_ptr<RenderTarget> m_renderTarget;

    // --- VARIABLES DE SCENE ---
    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12RootSignature> uiRootSignature;
    ComPtr<ID3D12PipelineState> m_psoSolid;
    ComPtr<ID3D12PipelineState> m_psoWireframe;
    ComPtr<ID3D12PipelineState> uiPSO = nullptr;

    // --- SHADERS ---
    ComPtr<ID3DBlob> m_vsByteCode;
    ComPtr<ID3DBlob> m_psSolidByteCode;
    ComPtr<ID3DBlob> m_psWireframeByteCode;
    ComPtr<ID3DBlob> uiVS = nullptr;
    ComPtr<ID3DBlob> uiPS = nullptr;

    // --- PARTICULES ---
    ComPtr<ID3D12RootSignature> m_particleComputeRootSig;
    ComPtr<ID3D12PipelineState> m_psParticleCompute;
    ComPtr<ID3D12RootSignature> m_particleRenderRootSig;
    ComPtr<ID3D12PipelineState> m_psParticleRender;
    ComPtr<ID3DBlob>            m_particleCS;
    ComPtr<ID3DBlob>            m_particleVS;
    ComPtr<ID3DBlob>            m_particlePS;

    D3D12_GPU_VIRTUAL_ADDRESS m_currentPassCBAddress = 0;

    std::unordered_map<uint32_t, std::unique_ptr<GPUParticleSystem>> m_particleSystems;

    std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayout;
    std::vector<D3D12_INPUT_ELEMENT_DESC> uiInputLayout;

    UINT m_currFrameIndex = 0;

    // --- BUFFERS ---
    ComPtr<ID3D12Resource> m_instanceUploadBuffers[FrameCount];
    BYTE* m_instanceBufferMapped[FrameCount] = { nullptr, nullptr };
    UINT m_instanceBufferOffset = 0;

    ComPtr<ID3D12Resource> m_passCB[FrameCount];
    BYTE* m_passCBMapped[FrameCount] = { nullptr, nullptr };

    ComPtr<ID3D12Resource> m_lightsCB[FrameCount];
    BYTE* m_lightsCBMapped[FrameCount] = { nullptr, nullptr };

    ComPtr<ID3D12Resource> m_boneUploadBuffers[FrameCount];
    BYTE* m_boneBufferMapped[FrameCount] = { nullptr, nullptr };

    std::unique_ptr<UploadBuffer<UIConstants>> uiCB = nullptr;
};