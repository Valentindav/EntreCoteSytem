#include "BatchManager.h"

BatchManager::BatchManager()
{
}

BatchManager::~BatchManager()
{
    Clear();
}

void BatchManager::AddInstance(MeshHandle meshHandle, const InstanceData& data)
{
    if (meshHandle == kInvalidHandle)
        return;

    m_batches[meshHandle].instances.push_back(data);
}

void BatchManager::Clear()
{
    for (auto& pair : m_batches)
    {
        pair.second.instances.clear();
    }
}