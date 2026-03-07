#include "Loaders/M3DLoader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>

struct M3dSubset
{
    uint32_t Id = 0;
    uint32_t VertexStart = 0;
    uint32_t VertexCount = 0;
    uint32_t FaceStart = 0;
    uint32_t FaceCount = 0;
};

void M3DLoader::SkipMaterials(std::ifstream& fin, uint32_t numMaterials)
{
    std::string line;
    std::getline(fin, line); // "Materials {"
    for (uint32_t i = 0; i < numMaterials; ++i)
        std::getline(fin, line); // ignorer chaque ligne de matériau
}

void M3DLoader::ReadSubsetTable(std::ifstream& fin, uint32_t numSubsets, std::vector<M3dSubset>& subsets)
{
    subsets.resize(numSubsets);
    std::string line;
    std::getline(fin, line); // "SubsetTable {"

    for (uint32_t i = 0; i < numSubsets; ++i)
    {
        std::getline(fin, line);
        std::stringstream ss(line);
        std::string ignore;
        ss >> ignore >> subsets[i].Id
            >> ignore >> subsets[i].VertexStart
            >> ignore >> subsets[i].VertexCount
            >> ignore >> subsets[i].FaceStart
            >> ignore >> subsets[i].FaceCount;
    }
}

void M3DLoader::ReadVertices(std::ifstream& fin, uint32_t numVertices, std::vector<Vertex>& outVertices, uint32_t numBones)
{
    outVertices.resize(numVertices);
    std::string line;
    std::getline(fin, line); // "Vertices {"

    for (uint32_t i = 0; i < numVertices; ++i)
    {
        std::getline(fin, line);
        std::stringstream ss(line);

        // Position
        ss >> outVertices[i].Pos.x >> outVertices[i].Pos.y >> outVertices[i].Pos.z;

        // Ignorer tangente (4 floats)
        float tmp;
        for (int j = 0; j < 4; ++j) ss >> tmp;

        // Normale
        ss >> outVertices[i].Normal.x >> outVertices[i].Normal.y >> outVertices[i].Normal.z;

        // UV
        ss >> outVertices[i].TexC.x >> outVertices[i].TexC.y;

        // Si skinned, ignorer poids et indices
        if (numBones > 0)
            for (int j = 0; j < 10; ++j) ss >> tmp;
    }
}

void M3DLoader::ReadTriangles(std::ifstream& fin, uint32_t numTriangles, std::vector<uint16_t>& outIndices)
{
    outIndices.resize(numTriangles * 3);
    std::string line;
    std::getline(fin, line); // "Triangles {"

    for (uint32_t i = 0; i < numTriangles; ++i)
    {
        std::getline(fin, line);
        std::stringstream ss(line);
        uint32_t a, b, c;
        ss >> a >> b >> c;
        outIndices[i * 3 + 0] = static_cast<uint16_t>(a);
        outIndices[i * 3 + 1] = static_cast<uint16_t>(b);
        outIndices[i * 3 + 2] = static_cast<uint16_t>(c);
    }
}

MeshData M3DLoader::LoadM3DMesh(const std::string& filename)
{
    std::ifstream fin(filename);
    if (!fin.is_open())
        throw std::runtime_error("Impossible d'ouvrir le fichier M3D : " + filename);

    MeshData mesh;
    mesh.Name = filename;

    std::string ignore;
    size_t numMaterials = 0, numVertices = 0, numTriangles = 0, numBones = 0, numAnimationClips = 0;

    // Lambda de sécurité (ultra robuste pour éviter les std::bad_alloc)
    auto skipTo = [](std::ifstream& stream, const std::string& keyword) {
        std::string token;
        while (stream >> token) {
            if (token.find(keyword) != std::string::npos) return true;
        }
        return false;
        };

    // --- 1. Header ---
    if (!skipTo(fin, "m3d-File-Header")) throw std::runtime_error("Fichier M3D invalide (Header introuvable)");
    fin >> ignore >> numMaterials;
    fin >> ignore >> numVertices;
    fin >> ignore >> numTriangles;
    fin >> ignore >> numBones;
    fin >> ignore >> numAnimationClips;

    // --- 2. Materials ---
    if (!skipTo(fin, "Materials")) throw std::runtime_error("Fichier M3D invalide (Materials introuvables)");
    for (size_t i = 0; i < numMaterials; ++i)
    {
        std::string dummyStr; float dummyF; bool dummyB;
        fin >> ignore >> dummyStr;                   // Name
        fin >> ignore >> dummyF >> dummyF >> dummyF; // Diffuse
        fin >> ignore >> dummyF >> dummyF >> dummyF; // Fresnel0
        fin >> ignore >> dummyF;                     // Roughness
        fin >> ignore >> dummyB;                     // AlphaClip
        fin >> ignore >> dummyStr;                   // MaterialTypeName
        fin >> ignore >> dummyStr;                   // DiffuseMap
        fin >> ignore >> dummyStr;                   // NormalMap
    }

    // --- 3. Subsets ---
    if (!skipTo(fin, "SubsetTable")) throw std::runtime_error("Fichier M3D invalide (SubsetTable introuvable)");
    for (size_t i = 0; i < numMaterials; ++i)
    {
        size_t id, vStart, vCount, fStart, fCount;
        fin >> ignore >> id;
        fin >> ignore >> vStart;
        fin >> ignore >> vCount;
        fin >> ignore >> fStart;
        fin >> ignore >> fCount;

        SubmeshData sub{};
        sub.BaseVertexLocation = vStart;
        sub.StartIndexLocation = fStart * 3;
        sub.IndexCount = fCount * 3;
        mesh.DrawArgs["subset_" + std::to_string(id)] = sub;
    }

    // --- 4. Vertices ---
    if (!skipTo(fin, "Vertices")) throw std::runtime_error("Fichier M3D invalide (Vertices introuvables)");
    mesh.Vertices.resize(numVertices);
    for (size_t i = 0; i < numVertices; ++i)
    {
        float px, py, pz, tx, ty, tz, tw, nx, ny, nz, u, v;

        fin >> ignore >> px >> py >> pz;
        fin >> ignore >> tx >> ty >> tz >> tw; // Tangente
        fin >> ignore >> nx >> ny >> nz;       // Normale
        fin >> ignore >> u >> v;               // UV

        // 🚨 C'EST ICI QUE TOUT SE JOUE POUR LE SKINNING
        if (numBones > 0)
        {
            float w0, w1, w2, w3;
            int i0, i1, i2, i3;
            fin >> ignore >> w0 >> w1 >> w2 >> w3;
            fin >> ignore >> i0 >> i1 >> i2 >> i3;

            // On ne stocke que les 3 premiers poids (le 4ème est calculé dans le shader : 1.0 - w0 - w1 - w2)
            mesh.Vertices[i].BoneWeights = DirectX::XMFLOAT3(w0, w1, w2);

            // On stocke les 4 indices d'os (castés en uint8_t pour correspondre au format DXGI_FORMAT_R8G8B8A8_UINT)
            mesh.Vertices[i].BoneIndices[0] = static_cast<uint8_t>(i0);
            mesh.Vertices[i].BoneIndices[1] = static_cast<uint8_t>(i1);
            mesh.Vertices[i].BoneIndices[2] = static_cast<uint8_t>(i2);
            mesh.Vertices[i].BoneIndices[3] = static_cast<uint8_t>(i3);
        }
        else
        {
            // Sécurité : Si le mesh n'a pas d'os (ex: un mur, une caisse), on met des zéros
            mesh.Vertices[i].BoneWeights = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
            mesh.Vertices[i].BoneIndices[0] = 0;
            mesh.Vertices[i].BoneIndices[1] = 0;
            mesh.Vertices[i].BoneIndices[2] = 0;
            mesh.Vertices[i].BoneIndices[3] = 0;
        }

        // On assigne enfin la géométrie classique
        mesh.Vertices[i].Pos = DirectX::XMFLOAT3(px, py, pz);
        mesh.Vertices[i].Normal = DirectX::XMFLOAT3(nx, ny, nz);
        mesh.Vertices[i].TexC = DirectX::XMFLOAT2(u, v);
    }

    // --- 5. Triangles ---
    if (!skipTo(fin, "Triangles")) throw std::runtime_error("Fichier M3D invalide (Triangles introuvables)");
    mesh.Indices.resize(numTriangles * 3);
    for (size_t i = 0; i < numTriangles; ++i)
    {
        uint32_t i0, i1, i2;
        fin >> i0 >> i1 >> i2;

        // Note : M3D stocke les indices dans un ordre précis, on inverse i1 et i2 pour respecter 
        // le sens des aiguilles d'une montre (Clockwise) typique de DirectX.
        mesh.Indices[i * 3 + 0] = i0;
        mesh.Indices[i * 3 + 1] = i2;
        mesh.Indices[i * 3 + 2] = i1;
    }

    return mesh;
}

SkeletonData M3DLoader::LoadM3DSkeleton(const std::string& filename)
{
    std::ifstream fin(filename);
    if (!fin.is_open())
        throw std::runtime_error("Impossible d'ouvrir le fichier M3D pour le squelette : " + filename);

    SkeletonData data;
    size_t numBones = 0;
    size_t numAnimationClips = 0;

    // LAMBDA AMÉLIORÉE : On utilise .find() au lieu de ==
    // Cela permet de trouver "BoneOffsets" même s'il est écrit "***BoneOffsets***" dans le fichier !
    auto skipTo = [](std::ifstream& stream, const std::string& keyword) {
        std::string token;
        while (stream >> token) {
            if (token.find(keyword) != std::string::npos) return true;
        }
        return false;
        };

    // --- 1. Lecture de l'en-tête ---
    if (!skipTo(fin, "#Bones")) return data; // Si pas de squelette, on quitte
    fin >> numBones;

    if (!skipTo(fin, "#AnimationClips")) return data;
    fin >> numAnimationClips;

    if (numBones == 0) return data;

    // --- 2. Lecture des Bone Offsets ---
    // Correction du mot-clé : "BoneOffsets" au lieu de "Bones"
    if (!skipTo(fin, "BoneOffsets"))
        throw std::runtime_error("Erreur M3D : Impossible de trouver 'BoneOffsets'");

    data.BoneOffsets.resize(numBones);
    for (size_t i = 0; i < numBones; ++i)
    {
        std::string ignoreToken;
        fin >> ignoreToken; // Ignore "Bone0:"

        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                fin >> data.BoneOffsets[i].m[r][c];
            }
        }
    }

    // --- 3. Lecture de la hiérarchie des os ---
    // Correction du mot-clé : "BoneHierarchy" au lieu de "Bone-Hierarchy"
    if (!skipTo(fin, "BoneHierarchy"))
        throw std::runtime_error("Erreur M3D : Impossible de trouver 'BoneHierarchy'");

    data.BoneHierarchy.resize(numBones);
    for (size_t i = 0; i < numBones; ++i)
    {
        std::string ignoreToken;
        fin >> ignoreToken; // Ignore "Bone0:" 
        fin >> data.BoneHierarchy[i];
    }

    // --- 4. Lecture des Clips d'Animation ---
    // Correction du mot-clé : "AnimationClips" au lieu de "Animation-Clips"
    if (!skipTo(fin, "AnimationClips"))
        throw std::runtime_error("Erreur M3D : Impossible de trouver 'AnimationClips'");

    for (size_t c = 0; c < numAnimationClips; ++c)
    {
        if (!skipTo(fin, "AnimationClip"))
            throw std::runtime_error("Erreur M3D : Impossible de trouver 'AnimationClip'");

        std::string clipName;
        fin >> clipName; // Ex: "Take1"

        AnimationClip clip;
        clip.BoneAnimations.resize(numBones);

        for (size_t b = 0; b < numBones; ++b)
        {
            if (!skipTo(fin, "#Keyframes:"))
                throw std::runtime_error("Erreur M3D : Impossible de trouver '#Keyframes:' pour l'os " + std::to_string(b));

            size_t numKeyframes = 0;
            fin >> numKeyframes;

            if (numKeyframes > 100000)
                throw std::runtime_error("Erreur M3D : Nombre de keyframes absurde (" + std::to_string(numKeyframes) + ")");

            skipTo(fin, "{"); // Ouvre le bloc Keyframes

            clip.BoneAnimations[b].Keyframes.resize(numKeyframes);
            for (size_t k = 0; k < numKeyframes; ++k)
            {
                float t;
                DirectX::XMFLOAT3 p, s;
                DirectX::XMFLOAT4 q;
                std::string ignoreWord;

                fin >> ignoreWord >> t;
                fin >> ignoreWord >> p.x >> p.y >> p.z;
                fin >> ignoreWord >> s.x >> s.y >> s.z;
                fin >> ignoreWord >> q.x >> q.y >> q.z >> q.w;

                clip.BoneAnimations[b].Keyframes[k] = { t, p, s, q };
            }
        }

        // On stocke le clip complet dans le dictionnaire
        data.Animations[clipName] = clip;
    }

    return data;
}