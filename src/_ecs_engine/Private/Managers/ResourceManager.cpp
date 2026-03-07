#include "ResourceManager.h"
#include "Private/EngineCore.h"

#include "GpuBufferConverter.h" 
#include "GpuCommandDispatcher.h" 
#include "GeometryFactory.h"
#include "Loaders/M3DLoader.h"
#include "Loaders/LoaderObj.h"
#include <iostream>

void ResourceManager::Initialize(GpuBufferConverter* gpuConverter, GpuCommandDispatcher* cmdDispatcher, std::mutex* graphicMutex)
{
    m_graphicMutex = graphicMutex;

    m_gpuBufferConverter = gpuConverter;
    m_gpuCommandDispatcher = cmdDispatcher;

    auto cmdList = m_gpuCommandDispatcher->ResetCommandList();

    uint32_t whitePixel = 0xFFFFFFFF; // RGBA : 255, 255, 255, 255
    TextureHandle defaultHandle = m_gpuBufferConverter->UploadTexture(
        cmdList,
        &whitePixel,
        1, 1,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        "default"
    );

    m_gpuCommandDispatcher->ExecuteCommandList();
    m_gpuCommandDispatcher->Flush();

    m_textureCache["default"] = defaultHandle;
}

MeshHandle ResourceManager::GetMeshHandle(const std::string& identifier)
{
    auto it = m_meshCache.find(identifier);
    if (it != m_meshCache.end()) {
        return it->second;
    }

    MeshData data = LoadMeshData(identifier);
    if (data.Vertices.empty()) return kInvalidHandle;

    std::lock_guard<std::mutex> lock(*m_graphicMutex);

    auto cmdList = m_gpuCommandDispatcher->ResetCommandList();
    MeshHandle newHandle = m_gpuBufferConverter->UploadMesh(cmdList, data);
    m_gpuCommandDispatcher->ExecuteCommandList();
    m_gpuCommandDispatcher->Flush();

    m_meshCache[identifier] = newHandle;
    return newHandle;
}

TextureHandle ResourceManager::GetTextureHandle(const std::string& identifier)
{
    auto it = m_textureCache.find(identifier);
    if (it != m_textureCache.end()) {
        return it->second;
    }

    std::lock_guard<std::mutex> lock(*m_graphicMutex);

    auto cmdList = m_gpuCommandDispatcher->ResetCommandList();
    TextureHandle newHandle = m_gpuBufferConverter->LoadTextureFromFile(cmdList, identifier);
    m_gpuCommandDispatcher->ExecuteCommandList();
    m_gpuCommandDispatcher->Flush();


    if (newHandle == kInvalidHandle) {
        return m_textureCache["default"];
    }

    m_textureCache[identifier] = newHandle;
    return newHandle;
}

SkeletonHandle ResourceManager::GetSkeletonHandle(const std::string& identifier)
{
    auto it = m_skeletonCache.find(identifier);
    if (it != m_skeletonCache.end()) {
        return it->second;
    }

    SkeletonData data = LoadSkeletonData(identifier);

    SkeletonHandle newHandle = static_cast<SkeletonHandle>(m_skeletonDataPool.size());
    m_skeletonDataPool.push_back(std::move(data));
    m_skeletonCache[identifier] = newHandle;

    return newHandle;
}

const SkeletonData* ResourceManager::GetSkeletonData(SkeletonHandle handle) const
{
    if (handle == kInvalidHandle || handle >= m_skeletonDataPool.size()) {
        return nullptr;
    }
    return &m_skeletonDataPool[handle];
}

MeshData ResourceManager::LoadMeshData(const std::string& identifier)
{
    MeshData data;
    data.Name = identifier;
    std::string ext = std::filesystem::path(identifier).extension().string();

    if (identifier == "cube") {
        data = Geometry::CreateBox(1.0f, 1.0f, 1.0f, 1);
    }
    else if (identifier == "sphere") {
        data = Geometry::CreateSphere(0.5f, 20, 20);
    }
    else if (identifier == "cylinder") {
        data = Geometry::CreateCylinder(0.5f, 0.5f, 1.f, 10, 10);
    }
    else if (identifier == "grid") {
        data = Geometry::CreateGrid(20.0f, 30.0f, 10, 10);
    }
    else if (identifier == "geosphere") {
        data = Geometry::CreateGeosphere(0.5f, 20);
    }
    else if (std::filesystem::exists(identifier) && ext == ".m3d") {
        data = M3DLoader::LoadM3DMesh(identifier);
    }
    else if (std::filesystem::exists(identifier) && ext == ".obj") {
        data = LoaderObj::Load(identifier);
    }

    return data;
}

SkeletonData ResourceManager::LoadSkeletonData(const std::string& identifier)
{
    SkeletonData data;
    std::string ext = std::filesystem::path(identifier).extension().string();

    if (std::filesystem::exists(identifier) && ext == ".m3d") {
        data = M3DLoader::LoadM3DSkeleton(identifier);
    }

    return data;
}
