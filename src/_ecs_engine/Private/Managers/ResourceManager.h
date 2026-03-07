#pragma once
#include <string>
#include <unordered_map>
#include <filesystem>
#include <GpuTypes.h>
#include <Data/MeshData.h>
#include <Data/AnimationData.h>
#include <mutex>

class GpuBufferConverter;
class GpuCommandDispatcher;

class ResourceManager
{
public:
    ResourceManager() = default;
    ~ResourceManager() = default;

    void Initialize(GpuBufferConverter* gpuConverter, GpuCommandDispatcher* cmdDispatcher, std::mutex* graphicMutex);

    MeshHandle GetMeshHandle(const std::string& filename);
    TextureHandle GetTextureHandle(const std::string& identifier);
    SkeletonHandle GetSkeletonHandle(const std::string& filename);
    const SkeletonData* GetSkeletonData(SkeletonHandle handle) const;

private:
    std::unordered_map<std::string, MeshHandle> m_meshCache;
    std::unordered_map<std::string, TextureHandle> m_textureCache;
    std::unordered_map<std::string, SkeletonHandle> m_skeletonCache;
    std::vector<SkeletonData> m_skeletonDataPool;

    GpuBufferConverter* m_gpuBufferConverter = nullptr;
    GpuCommandDispatcher* m_gpuCommandDispatcher = nullptr;

    MeshData LoadMeshData(const std::string& identifier);
    SkeletonData LoadSkeletonData(const std::string& identifier);

    std::mutex* m_graphicMutex;
};