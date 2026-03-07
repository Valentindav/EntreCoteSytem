#include "GPUParticleSystem.h" // N'oublie pas de changer le nom de l'include !
#include <cstring>
#pragma comment(lib, "d3d12.lib")

static HRESULT MakeBuffer(ID3D12Device* dev, UINT64 size, D3D12_HEAP_TYPE heap,
    D3D12_RESOURCE_STATES state, D3D12_RESOURCE_FLAGS flags, ID3D12Resource** out)
{
    D3D12_HEAP_PROPERTIES hp = {};
    hp.Type = heap;
    D3D12_RESOURCE_DESC rd = {};
    rd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    rd.Width = size;
    rd.Height = 1;
    rd.DepthOrArraySize = 1;
    rd.MipLevels = 1;
    rd.SampleDesc.Count = 1;
    rd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    rd.Flags = flags;
    return dev->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &rd, state, nullptr, IID_PPV_ARGS(out));
}

static UINT Align256(UINT n) { return (n + 255) & ~255; }

bool GPUParticleSystem::Init(
    ID3D12Device* device,
    ID3D12RootSignature** outComputeRootSig,
    ID3D12PipelineState** outComputePSO,
    ID3DBlob* csBlob,
    D3D12_CPU_DESCRIPTOR_HANDLE heapCpuStart,
    D3D12_GPU_DESCRIPTOR_HANDLE heapGpuStart,
    UINT                        descriptorSize,
    UINT                        baseSlot,
    uint32_t                    maxParticles)
{
    m_maxParticles = maxParticles;
    UINT64 bufSize = (UINT64)sizeof(GPUParticle) * maxParticles;
    D3D12_RANGE rNone = { 0, 0 };
    HRESULT hr;

    // 1. Buffer GPU (commence en COPY_DEST pour l'init)
    hr = MakeBuffer(device, bufSize, D3D12_HEAP_TYPE_DEFAULT,
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
        m_buffer.GetAddressOf());
    if (FAILED(hr)) return false;
    m_buffer->SetName(L"GPUParticleBuffer");

    // 2. Upload buffer (particules mortes)
    hr = MakeBuffer(device, bufSize, D3D12_HEAP_TYPE_UPLOAD,
        D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_FLAG_NONE,
        m_uploadInit.GetAddressOf());
    if (FAILED(hr)) return false;

    GPUParticle* mapped = nullptr;
    m_uploadInit->Map(0, &rNone, reinterpret_cast<void**>(&mapped));
    memset(mapped, 0, bufSize);
    m_uploadInit->Unmap(0, nullptr);

    // 3. CB compute
    hr = MakeBuffer(device, Align256(sizeof(GPUParticleParams)),
        D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ,
        D3D12_RESOURCE_FLAG_NONE, m_computeCB.GetAddressOf());
    if (FAILED(hr)) return false;
    m_computeCB->Map(0, &rNone, &m_computeCBMapped);

    // 4. CB render
    hr = MakeBuffer(device, Align256(sizeof(GPUParticleRenderCB)),
        D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ,
        D3D12_RESOURCE_FLAG_NONE, m_renderCB.GetAddressOf());
    if (FAILED(hr)) return false;
    m_renderCB->Map(0, &rNone, &m_renderCBMapped);

    // 5. UAV (slot baseSlot)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE cpuH = heapCpuStart;
        cpuH.ptr += (SIZE_T)(baseSlot * descriptorSize);

        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.Format = DXGI_FORMAT_UNKNOWN;
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.NumElements = maxParticles;
        uavDesc.Buffer.StructureByteStride = sizeof(GPUParticle);
        device->CreateUnorderedAccessView(m_buffer.Get(), nullptr, &uavDesc, cpuH);

        m_uavGpuHandle = heapGpuStart;
        m_uavGpuHandle.ptr += (SIZE_T)(baseSlot * descriptorSize);
    }

    // 6. SRV (slot baseSlot+1) — pour le vertex shader
    {
        D3D12_CPU_DESCRIPTOR_HANDLE cpuH = heapCpuStart;
        cpuH.ptr += (SIZE_T)((baseSlot + 1) * descriptorSize);

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Buffer.NumElements = maxParticles;
        srvDesc.Buffer.StructureByteStride = sizeof(GPUParticle);
        device->CreateShaderResourceView(m_buffer.Get(), &srvDesc, cpuH);

        m_srvGpuHandle = heapGpuStart;
        m_srvGpuHandle.ptr += (SIZE_T)((baseSlot + 1) * descriptorSize);
    }

    // 7. Compute root signature
    {
        D3D12_DESCRIPTOR_RANGE uavRange = {};
        uavRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
        uavRange.NumDescriptors = 1;
        uavRange.BaseShaderRegister = 0;

        D3D12_ROOT_PARAMETER params[2] = {};
        params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        params[0].Descriptor.ShaderRegister = 0;
        params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        params[1].DescriptorTable.NumDescriptorRanges = 1;
        params[1].DescriptorTable.pDescriptorRanges = &uavRange;
        params[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
        rsDesc.NumParameters = 2;
        rsDesc.pParameters = params;

        ComPtr<ID3DBlob> blob, err;
        hr = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1,
            blob.GetAddressOf(), err.GetAddressOf());
        if (FAILED(hr)) return false;
        hr = device->CreateRootSignature(0, blob->GetBufferPointer(),
            blob->GetBufferSize(), IID_PPV_ARGS(outComputeRootSig));
        if (FAILED(hr)) return false;
    }

    // 8. Compute PSO
    {
        if (!csBlob) return false;
        D3D12_COMPUTE_PIPELINE_STATE_DESC pso = {};
        pso.pRootSignature = *outComputeRootSig;
        pso.CS.pShaderBytecode = csBlob->GetBufferPointer();
        pso.CS.BytecodeLength = csBlob->GetBufferSize();
        hr = device->CreateComputePipelineState(&pso, IID_PPV_ARGS(outComputePSO));
        if (FAILED(hr)) return false;
    }

    return true;
}

void GPUParticleSystem::UploadInit(ID3D12GraphicsCommandList* cmdList)
{
    cmdList->CopyBufferRegion(m_buffer.Get(), 0, m_uploadInit.Get(), 0,
        (UINT64)sizeof(GPUParticle) * m_maxParticles);

    D3D12_RESOURCE_BARRIER b = {};
    b.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    b.Transition.pResource = m_buffer.Get();
    b.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    b.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    b.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    cmdList->ResourceBarrier(1, &b);

    m_currentState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
}

void GPUParticleSystem::Dispatch(
    ID3D12GraphicsCommandList* cmdList,
    ID3D12RootSignature* rootSig,
    ID3D12PipelineState* pso,
    ID3D12DescriptorHeap* heap,
    const GPUParticleParams& params)
{
    memcpy(m_computeCBMapped, &params, sizeof(params));

    if (m_currentState != D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
    {
        D3D12_RESOURCE_BARRIER b = {};
        b.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        b.Transition.pResource = m_buffer.Get();
        b.Transition.StateBefore = m_currentState;
        b.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        b.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        cmdList->ResourceBarrier(1, &b);
        m_currentState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    }

    cmdList->SetDescriptorHeaps(1, &heap);
    cmdList->SetComputeRootSignature(rootSig);
    cmdList->SetPipelineState(pso);
    cmdList->SetComputeRootConstantBufferView(0, m_computeCB->GetGPUVirtualAddress());
    cmdList->SetComputeRootDescriptorTable(1, m_uavGpuHandle);

    UINT groups = (m_maxParticles + THREAD_GROUP_SZ - 1) / THREAD_GROUP_SZ;
    cmdList->Dispatch(groups, 1, 1);

    D3D12_RESOURCE_BARRIER barriers[2] = {};
    barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    barriers[0].UAV.pResource = m_buffer.Get();

    barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barriers[1].Transition.pResource = m_buffer.Get();
    barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    barriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    cmdList->ResourceBarrier(2, barriers);
    m_currentState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
}

void GPUParticleSystem::Draw(
    ID3D12GraphicsCommandList* cmdList,
    ID3D12RootSignature* renderRootSig,
    ID3D12PipelineState* renderPSO,
    ID3D12DescriptorHeap* heap,
    const GPUParticleRenderCB& cb)
{
    memcpy(m_renderCBMapped, &cb, sizeof(cb));

    cmdList->SetDescriptorHeaps(1, &heap);
    cmdList->SetPipelineState(renderPSO);
    cmdList->SetGraphicsRootConstantBufferView(1, m_renderCB->GetGPUVirtualAddress());
    cmdList->SetGraphicsRootDescriptorTable(2, m_srvGpuHandle);

    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmdList->IASetVertexBuffers(0, 0, nullptr);
    cmdList->IASetIndexBuffer(nullptr);

    cmdList->DrawInstanced(m_maxParticles * 6, 1, 0, 0);
}

void GPUParticleSystem::Shutdown()
{
    if (m_computeCBMapped) { m_computeCB->Unmap(0, nullptr); m_computeCBMapped = nullptr; }
    if (m_renderCBMapped) { m_renderCB->Unmap(0, nullptr);  m_renderCBMapped = nullptr; }
    m_buffer.Reset();
    m_uploadInit.Reset();
    m_computeCB.Reset();
    m_renderCB.Reset();
}