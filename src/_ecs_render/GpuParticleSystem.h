#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include <cstdint>

using Microsoft::WRL::ComPtr;

// Doit correspondre exactement au struct HLSL
struct GPUParticle
{
    float position[3];
    float life;
    float velocity[3];
    float maxLife;
    float pad[2];   // => 40 bytes, multiple de 4
};

// CB envoye au compute shader (b0)
// Doit etre aligne sur 256 bytes, CalcConstantBufferByteSize s'en charge
struct GPUParticleParams
{
    float    deltaTime;
    float    gravity;
    float    originX, originY, originZ;
    float    spawnRadius;
    float    speed;
    float    spread;
    float    minLife;
    float    maxLife;
    float    sizeStart;
    float    sizeEnd;
    float    colorStartR, colorStartG, colorStartB, colorStartA;
    float    colorEndR, colorEndG, colorEndB, colorEndA;
    uint32_t maxParticles;
    float    emitRate;
    float    deltaAccum;
    float    pad;
};

// CB envoye au vertex shader (b1)
// Alignement : 2 float, puis 2 float4 => 2*4 + 2*16 = 40 bytes, ok
struct GPUParticleRenderCB
{
    float  sizeStart;
    float  sizeEnd;
    float  pad0;
    float  pad1;
    float  colorStartR, colorStartG, colorStartB, colorStartA;
    float  colorEndR, colorEndG, colorEndB, colorEndA;
};

class GPUParticleSystem
{
public:
    static constexpr uint32_t THREAD_GROUP_SZ = 64;

    bool Init(
        ID3D12Device* device,
        ID3D12RootSignature** outComputeRootSig,
        ID3D12PipelineState** outComputePSO,
        ID3DBlob* csBlob,
        D3D12_CPU_DESCRIPTOR_HANDLE heapCpuStart,
        D3D12_GPU_DESCRIPTOR_HANDLE heapGpuStart,
        UINT                        descriptorSize,
        UINT                        baseSlot,
        uint32_t                    maxParticles
    );

    void UploadInit(ID3D12GraphicsCommandList* cmdList);

    void Dispatch(
        ID3D12GraphicsCommandList* cmdList,
        ID3D12RootSignature* rootSig,
        ID3D12PipelineState* pso,
        ID3D12DescriptorHeap* heap,
        const GPUParticleParams& params
    );

    void Draw(
        ID3D12GraphicsCommandList* cmdList,
        ID3D12RootSignature* renderRootSig,
        ID3D12PipelineState* renderPSO,
        ID3D12DescriptorHeap* heap,
        const GPUParticleRenderCB& cb
    );

    uint32_t GetMaxParticles() const { return m_maxParticles; }

    void Shutdown();

private:
    uint32_t m_maxParticles = 0;

    D3D12_RESOURCE_STATES m_currentState = D3D12_RESOURCE_STATE_COMMON;

    ComPtr<ID3D12Resource> m_buffer;
    ComPtr<ID3D12Resource> m_uploadInit;

    ComPtr<ID3D12Resource> m_computeCB;
    void* m_computeCBMapped = nullptr;

    ComPtr<ID3D12Resource> m_renderCB;
    void* m_renderCBMapped = nullptr;

    D3D12_GPU_DESCRIPTOR_HANDLE m_uavGpuHandle = {};
    D3D12_GPU_DESCRIPTOR_HANDLE m_srvGpuHandle = {};
};