#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <cstdint>
#include "Data/MeshData.h"
#include "Data/AnimationData.h"

class M3DLoader
{
public:
    static MeshData LoadM3DMesh(const std::string& filename);
    static SkeletonData LoadM3DSkeleton(const std::string& filename);

private:
    struct M3dSubset {
        uint32_t Id;
        uint32_t VertexStart;
        uint32_t VertexCount;
        uint32_t FaceStart;
        uint32_t FaceCount;
    };

    static void SkipMaterials(std::ifstream& fin, uint32_t numMaterials);
    static void ReadSubsetTable(std::ifstream& fin, uint32_t numSubsets, std::vector<M3dSubset>& subsets);
    static void ReadVertices(std::ifstream& fin, uint32_t numVertices, std::vector<Vertex>& outVertices, uint32_t numBones);
    static void ReadTriangles(std::ifstream& fin, uint32_t numTriangles, std::vector<uint16_t>& outIndices);
};
