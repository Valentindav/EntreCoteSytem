#pragma once

#include <string>
#include <vector>
#include <DirectXMath.h>
#include <unordered_map>
#include <DirectXCollision.h>

struct Vertex {
    DirectX::XMFLOAT3 Pos;         // offset 0
    DirectX::XMFLOAT3 Normal;      // offset 12
    DirectX::XMFLOAT2 TexC;        // offset 24
    DirectX::XMFLOAT3 BoneWeights; // offset 32
    uint8_t BoneIndices[4];        // offset 44
};

struct SubmeshData
{
    uint32_t IndexCount = 0;
    uint32_t StartIndexLocation = 0;
    int32_t BaseVertexLocation = 0;
    DirectX::BoundingBox Bounds;
};

struct MeshData
{
    std::string Name;

    std::vector<Vertex> Vertices;
    std::vector<uint32_t> Indices;

    std::unordered_map<std::string, SubmeshData> DrawArgs;

    size_t GetVertexBufferSize() const { return Vertices.size() * sizeof(Vertex); }
    size_t GetIndexBufferSize() const { return Indices.size() * sizeof(uint32_t); }
};