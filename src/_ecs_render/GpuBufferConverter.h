#pragma once

#include "GpuTypes.h"
#include "Data/MeshData.h"  
#include "d3dUtil.h"   
#include <vector>
#include <unordered_map>
#include <memory>

class GpuDevice;

struct MeshBuffers
{
    D3D12_VERTEX_BUFFER_VIEW VertexBufferView;
    D3D12_INDEX_BUFFER_VIEW IndexBufferView;
    UINT IndexCount = 0;
};

struct GpuMeshBuffer
{
    std::string Name;

    Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferGPU;
    Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferGPU;

    Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUploader;
    Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUploader;

    D3D12_VERTEX_BUFFER_VIEW VBV;
    D3D12_INDEX_BUFFER_VIEW IBV;
    UINT IndexCount = 0;

    std::unordered_map<std::string, SubmeshData> DrawArgs;

    void CreateViews(size_t vbSize, size_t vbStride, size_t ibSize, DXGI_FORMAT indexFormat, UINT indexCount)
    {
        VBV.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
        VBV.StrideInBytes = static_cast<UINT>(vbStride);
        VBV.SizeInBytes = static_cast<UINT>(vbSize);

        IBV.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
        IBV.Format = indexFormat;
        IBV.SizeInBytes = static_cast<UINT>(ibSize);

        IndexCount = indexCount;
    }
};

struct GpuTexture
{
    std::string Name;
    Microsoft::WRL::ComPtr<ID3D12Resource> Resource;
    Microsoft::WRL::ComPtr<ID3D12Resource> UploadHeap;

    UINT SrvHeapIndex = 0;
};

class GpuBufferConverter
{
public:
    GpuBufferConverter(GpuDevice* device);
    ~GpuBufferConverter();

    MeshHandle UploadMesh(ID3D12GraphicsCommandList* cmdList, const MeshData& meshData);
    TextureHandle UploadTexture(ID3D12GraphicsCommandList* cmdList, const void* data, UINT width, UINT height, DXGI_FORMAT format, const std::string& name);
    TextureHandle LoadTextureFromFile(ID3D12GraphicsCommandList* cmdList, const std::string& filepath);

    const GpuMeshBuffer* GetMesh(MeshHandle handle) const;

    MeshBuffers GetMeshBuffers(MeshHandle handle) const;

    UINT GetTextureSrvIndex(TextureHandle handle) const {
        if (handle < mTextures.size()) return mTextures[handle]->SrvHeapIndex;
        return 0;
    }

private:
    GpuDevice* mDevice;
    std::vector<std::unique_ptr<GpuMeshBuffer>> mMeshes;
    std::vector<std::unique_ptr<GpuTexture>> mTextures;
};