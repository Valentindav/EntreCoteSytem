#include "GeometryFactory.h"
#include <algorithm>
#include <cmath>
#include <vector>

using namespace DirectX;

namespace Geometry
{
    // Helper pour remplir un sommet avec Position + Normale + UV
    void AddVertex(MeshData& mesh, const XMFLOAT3& p, const XMFLOAT3& n, const XMFLOAT2& uv)
    {
        Vertex v;
        v.Pos = p;
        v.Normal = n;
        v.TexC = uv;
        mesh.Vertices.push_back(v);
    }

    void FinalizeMesh(MeshData& mesh, const std::string& submeshName)
    {
        SubmeshData submesh;
        submesh.IndexCount = (uint32_t)mesh.Indices.size();
        submesh.StartIndexLocation = 0;
        submesh.BaseVertexLocation = 0;

        XMFLOAT3 vMin = { FLT_MAX, FLT_MAX, FLT_MAX };
        XMFLOAT3 vMax = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

        for (const auto& v : mesh.Vertices)
        {
            vMin.x = std::min(vMin.x, v.Pos.x);
            vMin.y = std::min(vMin.y, v.Pos.y);
            vMin.z = std::min(vMin.z, v.Pos.z);

            vMax.x = std::max(vMax.x, v.Pos.x);
            vMax.y = std::max(vMax.y, v.Pos.y);
            vMax.z = std::max(vMax.z, v.Pos.z);
        }

        submesh.Bounds.Center = XMFLOAT3(0.5f * (vMin.x + vMax.x), 0.5f * (vMin.y + vMax.y), 0.5f * (vMin.z + vMax.z));
        submesh.Bounds.Extents = XMFLOAT3(0.5f * (vMax.x - vMin.x), 0.5f * (vMax.y - vMin.y), 0.5f * (vMax.z - vMin.z));

        mesh.DrawArgs[submeshName] = submesh;
    }

    MeshData CreateBox(float width, float height, float depth, uint32_t numSubdivisions)
    {
        MeshData mesh;
        mesh.Name = "Box";

        float w2 = 0.5f * width;
        float h2 = 0.5f * height;
        float d2 = 0.5f * depth;

        // --- DEFINITION DES VERTICES (POS, NORMAL, UV) ---
        // Règle d'or : On regarde toujours la face de l'extérieur.
        // Ordre : Bas-Gauche(0), Haut-Gauche(1), Haut-Droit(2), Bas-Droit(3)

        // Face Avant (+Z) - Normale (0, 0, 1)
        // Vue de face : +X est à gauche, -X est à droite, +Y est en haut
        AddVertex(mesh, XMFLOAT3(w2, -h2, d2), XMFLOAT3(0, 0, 1), XMFLOAT2(0, 1)); // Bas Gauche
        AddVertex(mesh, XMFLOAT3(w2, h2, d2), XMFLOAT3(0, 0, 1), XMFLOAT2(0, 0)); // Haut Gauche
        AddVertex(mesh, XMFLOAT3(-w2, h2, d2), XMFLOAT3(0, 0, 1), XMFLOAT2(1, 0)); // Haut Droit
        AddVertex(mesh, XMFLOAT3(-w2, -h2, d2), XMFLOAT3(0, 0, 1), XMFLOAT2(1, 1)); // Bas Droit

        // Face Arrière (-Z) - Normale (0, 0, -1)
        // Vue de dos : -X est à gauche, +X est à droite
        AddVertex(mesh, XMFLOAT3(-w2, -h2, -d2), XMFLOAT3(0, 0, -1), XMFLOAT2(0, 1));
        AddVertex(mesh, XMFLOAT3(-w2, h2, -d2), XMFLOAT3(0, 0, -1), XMFLOAT2(0, 0));
        AddVertex(mesh, XMFLOAT3(w2, h2, -d2), XMFLOAT3(0, 0, -1), XMFLOAT2(1, 0));
        AddVertex(mesh, XMFLOAT3(w2, -h2, -d2), XMFLOAT3(0, 0, -1), XMFLOAT2(1, 1));

        // Face Haut (+Y) - Normale (0, 1, 0)
        // Vue de dessus : -Z est en bas, +Z est en haut, -X est à gauche
        AddVertex(mesh, XMFLOAT3(-w2, h2, -d2), XMFLOAT3(0, 1, 0), XMFLOAT2(0, 1));
        AddVertex(mesh, XMFLOAT3(-w2, h2, d2), XMFLOAT3(0, 1, 0), XMFLOAT2(0, 0));
        AddVertex(mesh, XMFLOAT3(w2, h2, d2), XMFLOAT3(0, 1, 0), XMFLOAT2(1, 0));
        AddVertex(mesh, XMFLOAT3(w2, h2, -d2), XMFLOAT3(0, 1, 0), XMFLOAT2(1, 1));

        // Face Bas (-Y) - Normale (0, -1, 0)
        // Vue de dessous : +Z est en bas, -Z est en haut, -X est à gauche
        AddVertex(mesh, XMFLOAT3(-w2, -h2, d2), XMFLOAT3(0, -1, 0), XMFLOAT2(0, 1));
        AddVertex(mesh, XMFLOAT3(-w2, -h2, -d2), XMFLOAT3(0, -1, 0), XMFLOAT2(0, 0));
        AddVertex(mesh, XMFLOAT3(w2, -h2, -d2), XMFLOAT3(0, -1, 0), XMFLOAT2(1, 0));
        AddVertex(mesh, XMFLOAT3(w2, -h2, d2), XMFLOAT3(0, -1, 0), XMFLOAT2(1, 1));

        // Face Gauche (-X) - Normale (-1, 0, 0)
        // Vue de gauche : +Z est à gauche, -Z est à droite
        AddVertex(mesh, XMFLOAT3(-w2, -h2, d2), XMFLOAT3(-1, 0, 0), XMFLOAT2(0, 1));
        AddVertex(mesh, XMFLOAT3(-w2, h2, d2), XMFLOAT3(-1, 0, 0), XMFLOAT2(0, 0));
        AddVertex(mesh, XMFLOAT3(-w2, h2, -d2), XMFLOAT3(-1, 0, 0), XMFLOAT2(1, 0));
        AddVertex(mesh, XMFLOAT3(-w2, -h2, -d2), XMFLOAT3(-1, 0, 0), XMFLOAT2(1, 1));

        // Face Droite (+X) - Normale (1, 0, 0)
        // Vue de droite : -Z est à gauche, +Z est à droite
        AddVertex(mesh, XMFLOAT3(w2, -h2, -d2), XMFLOAT3(1, 0, 0), XMFLOAT2(0, 1));
        AddVertex(mesh, XMFLOAT3(w2, h2, -d2), XMFLOAT3(1, 0, 0), XMFLOAT2(0, 0));
        AddVertex(mesh, XMFLOAT3(w2, h2, d2), XMFLOAT3(1, 0, 0), XMFLOAT2(1, 0));
        AddVertex(mesh, XMFLOAT3(w2, -h2, d2), XMFLOAT3(1, 0, 0), XMFLOAT2(1, 1));

        // Indices (2 triangles par face, tous dans le même ordre relatif)
        std::vector<uint16_t> indices = {
            0, 1, 2, 0, 2, 3,       // Avant (+Z)
            4, 5, 6, 4, 6, 7,       // Arrière (-Z)
            8, 9, 10, 8, 10, 11,    // Haut (+Y)
            12, 13, 14, 12, 14, 15, // Bas (-Y)
            16, 17, 18, 16, 18, 19, // Gauche (-X)
            20, 21, 22, 20, 22, 23  // Droite (+X)
        };

        mesh.Indices.assign(indices.begin(), indices.end());
        FinalizeMesh(mesh, "box");
        return mesh;
    }

    MeshData CreateSphere(float radius, uint32_t sliceCount, uint32_t stackCount)
    {
        MeshData mesh;
        mesh.Name = "Sphere";

        // 1. Sommet Pôle Nord
        AddVertex(mesh, XMFLOAT3(0.0f, radius, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.5f, 0.0f));

        float phiStep = XM_PI / stackCount;
        float thetaStep = 2.0f * XM_PI / sliceCount;

        // 2. Anneaux
        for (uint32_t i = 1; i <= stackCount - 1; ++i)
        {
            float phi = i * phiStep;

            for (uint32_t j = 0; j <= sliceCount; ++j)
            {
                float theta = j * thetaStep;

                XMFLOAT3 p;
                p.x = radius * sinf(phi) * cosf(theta);
                p.y = radius * cosf(phi);
                p.z = radius * sinf(phi) * sinf(theta);

                // La normale d'une sphère à l'origine est simplement la Position normalisée
                XMFLOAT3 n = p;
                XMVECTOR vn = XMLoadFloat3(&n);
                vn = XMVector3Normalize(vn);
                XMStoreFloat3(&n, vn);

                // Calcul UV
                XMFLOAT2 uv;
                uv.x = theta / XM_2PI;
                uv.y = phi / XM_PI;

                AddVertex(mesh, p, n, uv);
            }
        }

        // 3. Sommet Pôle Sud
        AddVertex(mesh, XMFLOAT3(0.0f, -radius, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.5f, 1.0f));

        // 4. Indices
        // Pôle Nord
        for (uint32_t i = 1; i <= sliceCount; ++i)
        {
            mesh.Indices.push_back(0);
            mesh.Indices.push_back(i + 1);
            mesh.Indices.push_back(i);
        }

        // Stacks intermédiaires
        uint32_t baseIndex = 1;
        uint32_t ringVertexCount = sliceCount + 1;

        for (uint32_t i = 0; i < stackCount - 2; ++i)
        {
            for (uint32_t j = 0; j < sliceCount; ++j)
            {
                mesh.Indices.push_back(baseIndex + i * ringVertexCount + j);
                mesh.Indices.push_back(baseIndex + i * ringVertexCount + j + 1);
                mesh.Indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);

                mesh.Indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);
                mesh.Indices.push_back(baseIndex + i * ringVertexCount + j + 1);
                mesh.Indices.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
            }
        }

        // Pôle Sud
        uint32_t southPoleIndex = (uint32_t)mesh.Vertices.size() - 1;
        baseIndex = southPoleIndex - ringVertexCount;

        for (uint32_t i = 0; i < sliceCount; ++i)
        {
            mesh.Indices.push_back(southPoleIndex);
            mesh.Indices.push_back(baseIndex + i);
            mesh.Indices.push_back(baseIndex + i + 1);
        }

        FinalizeMesh(mesh, "sphere");
        return mesh;
    }

    MeshData CreateGeosphere(float radius, uint32_t numSubdivisions)
    {
        return CreateSphere(radius, numSubdivisions * 4, numSubdivisions * 2);
    }

    MeshData CreateCylinder(float bottomRadius, float topRadius, float height, uint32_t sliceCount, uint32_t stackCount)
    {
        MeshData mesh;
        mesh.Name = "Cylinder";

        float stackHeight = height / stackCount;
        float radiusStep = (topRadius - bottomRadius) / stackCount;
        uint32_t ringCount = stackCount + 1;

        // 1. Anneaux (Flancs du cylindre)
        for (uint32_t i = 0; i < ringCount; ++i)
        {
            float y = -0.5f * height + i * stackHeight;
            float r = bottomRadius + i * radiusStep;
            float dTheta = 2.0f * XM_PI / sliceCount;

            for (uint32_t j = 0; j <= sliceCount; ++j)
            {
                float c = cosf(j * dTheta);
                float s = sinf(j * dTheta);

                XMFLOAT3 p = { r * c, y, r * s };

                // Normale latérale (ignorant la pente pour simplifier)
                XMFLOAT3 n = { c, 0.0f, s };

                // UVs
                XMFLOAT2 uv = { (float)j / sliceCount, 1.0f - (float)i / stackCount };

                AddVertex(mesh, p, n, uv);
            }
        }

        // 2. Indices des flancs
        uint32_t ringVertexCount = sliceCount + 1;
        for (uint32_t i = 0; i < stackCount; ++i)
        {
            for (uint32_t j = 0; j < sliceCount; ++j)
            {
                mesh.Indices.push_back(i * ringVertexCount + j);
                mesh.Indices.push_back((i + 1) * ringVertexCount + j);
                mesh.Indices.push_back((i + 1) * ringVertexCount + j + 1);

                mesh.Indices.push_back(i * ringVertexCount + j);
                mesh.Indices.push_back((i + 1) * ringVertexCount + j + 1);
                mesh.Indices.push_back(i * ringVertexCount + j + 1);
            }
        }

        // 3. Cap Haut
        uint32_t baseIndex = (uint32_t)mesh.Vertices.size();
        AddVertex(mesh, XMFLOAT3(0, 0.5f * height, 0), XMFLOAT3(0, 1, 0), XMFLOAT2(0.5f, 0.5f)); // Centre

        float y = 0.5f * height;
        float dTheta = 2.0f * XM_PI / sliceCount;
        for (uint32_t i = 0; i <= sliceCount; ++i)
        {
            float x = topRadius * cosf(i * dTheta);
            float z = topRadius * sinf(i * dTheta);

            // Mapping UV circulaire
            float u = x / height + 0.5f;
            float v = z / height + 0.5f;

            AddVertex(mesh, XMFLOAT3(x, y, z), XMFLOAT3(0, 1, 0), XMFLOAT2(u, v));
        }

        uint32_t centerIndex = baseIndex;
        for (uint32_t i = 0; i < sliceCount; ++i)
        {
            mesh.Indices.push_back(centerIndex);
            mesh.Indices.push_back(baseIndex + i + 1);
            mesh.Indices.push_back(baseIndex + i + 2);
        }

        // 4. Cap Bas
        baseIndex = (uint32_t)mesh.Vertices.size();
        AddVertex(mesh, XMFLOAT3(0, -0.5f * height, 0), XMFLOAT3(0, -1, 0), XMFLOAT2(0.5f, 0.5f)); // Centre

        y = -0.5f * height;
        for (uint32_t i = 0; i <= sliceCount; ++i)
        {
            float x = bottomRadius * cosf(i * dTheta);
            float z = bottomRadius * sinf(i * dTheta);

            float u = x / height + 0.5f;
            float v = z / height + 0.5f;

            AddVertex(mesh, XMFLOAT3(x, y, z), XMFLOAT3(0, -1, 0), XMFLOAT2(u, v));
        }

        centerIndex = baseIndex;
        for (uint32_t i = 0; i < sliceCount; ++i)
        {
            mesh.Indices.push_back(centerIndex);
            mesh.Indices.push_back(baseIndex + i + 2);
            mesh.Indices.push_back(baseIndex + i + 1);
        }

        FinalizeMesh(mesh, "cylinder");
        return mesh;
    }

    MeshData CreateGrid(float width, float depth, uint32_t m, uint32_t n)
    {
        MeshData mesh;
        mesh.Name = "Grid";

        float dx = width / (n - 1);
        float dz = depth / (m - 1);

        float du = 1.0f / (n - 1);
        float dv = 1.0f / (m - 1);

        // 1. Sommets
        for (uint32_t i = 0; i < m; ++i)
        {
            float z = 0.5f * depth - i * dz;
            for (uint32_t j = 0; j < n; ++j)
            {
                float x = -0.5f * width + j * dx;

                // Normale vers le haut (0, 1, 0)
                XMFLOAT3 normal = XMFLOAT3(0.0f, 1.0f, 0.0f);

                // UVs
                XMFLOAT2 uv = XMFLOAT2(j * du, i * dv);

                AddVertex(mesh, XMFLOAT3(x, 0, z), normal, uv);
            }
        }

        // 2. Indices
        for (uint32_t i = 0; i < m - 1; ++i)
        {
            for (uint32_t j = 0; j < n - 1; ++j)
            {
                mesh.Indices.push_back(i * n + j);
                mesh.Indices.push_back(i * n + j + 1);
                mesh.Indices.push_back((i + 1) * n + j);

                mesh.Indices.push_back((i + 1) * n + j);
                mesh.Indices.push_back(i * n + j + 1);
                mesh.Indices.push_back((i + 1) * n + j + 1);
            }
        }

        FinalizeMesh(mesh, "grid");
        return mesh;
    }

    MeshData CreateQuad(float x, float y, float w, float h, float depth)
    {
        MeshData mesh;
        mesh.Name = "Quad";

        float w2 = w * 0.5f;
        float h2 = h * 0.5f;

        // Normale vers l'arrière pour un quad 2D (0, 0, -1)
        XMFLOAT3 normal = XMFLOAT3(0.0f, 0.0f, -1.0f);

        AddVertex(mesh, XMFLOAT3(x - w2, y - h2, depth), normal, XMFLOAT2(0.0f, 1.0f)); // Bas-Gauche
        AddVertex(mesh, XMFLOAT3(x - w2, y + h2, depth), normal, XMFLOAT2(0.0f, 0.0f)); // Haut-Gauche
        AddVertex(mesh, XMFLOAT3(x + w2, y + h2, depth), normal, XMFLOAT2(1.0f, 0.0f)); // Haut-Droit
        AddVertex(mesh, XMFLOAT3(x + w2, y - h2, depth), normal, XMFLOAT2(1.0f, 1.0f)); // Bas-Droit

        mesh.Indices = { 0, 1, 2, 0, 2, 3 };

        FinalizeMesh(mesh, "quad");
        return mesh;
    }
}