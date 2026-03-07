#include "GpuBufferConverter.h"
#include "GpuDevice.h"
#include "Loaders/DDSTextureLoader.h"
#include "Loaders/WICTextureLoader.h"
#include <filesystem>

GpuBufferConverter::GpuBufferConverter(GpuDevice* device)
    : mDevice(device)
{
}

GpuBufferConverter::~GpuBufferConverter()
{
    mMeshes.clear();
}

MeshHandle GpuBufferConverter::UploadMesh(ID3D12GraphicsCommandList* cmdList, const MeshData& meshData)
{
    auto gpuMesh = std::make_unique<GpuMeshBuffer>();
    gpuMesh->Name = meshData.Name;
    gpuMesh->DrawArgs = meshData.DrawArgs;

    // 1. Upload des sommets (Reste inchangé)
    gpuMesh->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(
        mDevice->GetDevice(),
        cmdList,
        meshData.Vertices.data(),
        meshData.GetVertexBufferSize(),
        gpuMesh->VertexBufferUploader
    );

    DXGI_FORMAT indexFormat = DXGI_FORMAT_R32_UINT;

    gpuMesh->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(
        mDevice->GetDevice(),
        cmdList,
        meshData.Indices.data(),
        meshData.GetIndexBufferSize(),
        gpuMesh->IndexBufferUploader
    );

    gpuMesh->CreateViews(
        meshData.GetVertexBufferSize(),
        sizeof(Vertex),
        meshData.GetIndexBufferSize(),
        indexFormat,
        (UINT)meshData.Indices.size()
    );

    mMeshes.push_back(std::move(gpuMesh));
    return static_cast<MeshHandle>(mMeshes.size() - 1);
}

TextureHandle GpuBufferConverter::UploadTexture(ID3D12GraphicsCommandList* cmdList, const void* data, UINT width, UINT height, DXGI_FORMAT format, const std::string& name)
{
    auto gpuTex = std::make_unique<GpuTexture>();
    gpuTex->Name = name;

    // 1. Description de la texture 2D
    D3D12_RESOURCE_DESC texDesc = {};
    texDesc.MipLevels = 1;
    texDesc.Format = format;
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    texDesc.DepthOrArraySize = 1;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    // 2. Création de la ressource GPU principale (Default Heap)
    CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);
    mDevice->GetDevice()->CreateCommittedResource(
        &defaultHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&gpuTex->Resource));

    // 3. Création du Upload Buffer
    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(gpuTex->Resource.Get(), 0, 1);
    CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
    mDevice->GetDevice()->CreateCommittedResource(
        &uploadHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&gpuTex->UploadHeap));

    // 4. Copie des données
    D3D12_SUBRESOURCE_DATA subresourceData = {};
    subresourceData.pData = data;
    subresourceData.RowPitch = width * 4;
    subresourceData.SlicePitch = subresourceData.RowPitch * height;

    UpdateSubresources(cmdList, gpuTex->Resource.Get(), gpuTex->UploadHeap.Get(), 0, 0, 1, &subresourceData);

    // 5. Transition vers PIXEL_SHADER_RESOURCE
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        gpuTex->Resource.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    cmdList->ResourceBarrier(1, &barrier);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;

    D3D12_CPU_DESCRIPTOR_HANDLE srvHandle;
    gpuTex->SrvHeapIndex = mDevice->AllocateDescriptor(&srvHandle);
    mDevice->GetDevice()->CreateShaderResourceView(gpuTex->Resource.Get(), &srvDesc, srvHandle);

    mTextures.push_back(std::move(gpuTex));
    return static_cast<TextureHandle>(mTextures.size() - 1);
}

TextureHandle GpuBufferConverter::LoadTextureFromFile(ID3D12GraphicsCommandList* cmdList, const std::string& filepath)
{
    auto gpuTex = std::make_unique<GpuTexture>();
    gpuTex->Name = filepath;
    std::wstring wfilepath(filepath.begin(), filepath.end());

    std::unique_ptr<uint8_t[]> decodedData;
    D3D12_SUBRESOURCE_DATA subresource;

    HRESULT hr = DirectX::LoadWICTextureFromFile(
        mDevice->GetDevice(),
        wfilepath.c_str(),
        &gpuTex->Resource,
        decodedData,
        subresource
    );

    if (FAILED(hr)) return kInvalidHandle;

    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(gpuTex->Resource.Get(), 0, 1);

    auto uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

    mDevice->GetDevice()->CreateCommittedResource(
        &uploadHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&gpuTex->UploadHeap));

    UpdateSubresources(cmdList, gpuTex->Resource.Get(), gpuTex->UploadHeap.Get(), 0, 0, 1, &subresource);

    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        gpuTex->Resource.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    cmdList->ResourceBarrier(1, &barrier);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = gpuTex->Resource->GetDesc().Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;

    D3D12_CPU_DESCRIPTOR_HANDLE srvHandle;
    gpuTex->SrvHeapIndex = mDevice->AllocateDescriptor(&srvHandle);
    mDevice->GetDevice()->CreateShaderResourceView(gpuTex->Resource.Get(), &srvDesc, srvHandle);

    mTextures.push_back(std::move(gpuTex));
    return static_cast<TextureHandle>(mTextures.size() - 1);
}

const GpuMeshBuffer* GpuBufferConverter::GetMesh(MeshHandle handle) const
{
    if (handle >= mMeshes.size() || handle == kInvalidHandle)
    {
        return nullptr;
    }
    return mMeshes[handle].get();
}

MeshBuffers GpuBufferConverter::GetMeshBuffers(MeshHandle handle) const
{
    MeshBuffers result = {};

    if (handle == kInvalidHandle || handle >= mMeshes.size() || !mMeshes[handle])
    {
        return result;
    }

    const auto& mesh = mMeshes[handle];

    result.VertexBufferView = mesh->VBV;
    result.IndexBufferView = mesh->IBV;
    result.IndexCount = mesh->IndexCount;

    return result;
}