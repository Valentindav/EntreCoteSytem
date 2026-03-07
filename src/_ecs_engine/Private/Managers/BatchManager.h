#pragma once

#include <unordered_map>
#include <vector>
#include "GpuTypes.h"
#include "Data/BatchData.h"

class BatchManager
{
public:
    BatchManager();
    ~BatchManager();

    BatchManager(const BatchManager&) = delete;
    BatchManager& operator=(const BatchManager&) = delete;

    void AddInstance(MeshHandle meshHandle, const InstanceData& data);

    void Clear();

    const std::unordered_map<MeshHandle, BatchData>& GetBatches() const { return m_batches; }

    bool IsEmpty() const { return m_batches.empty(); }

private:
    std::unordered_map<MeshHandle, BatchData> m_batches;
};