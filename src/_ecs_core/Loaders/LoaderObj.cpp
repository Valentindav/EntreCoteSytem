#include "LoaderObj.h"
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include "DirectXMath.h"
#include "Data/MeshData.h"

MeshData LoaderObj::Load(const std::string& filename) {
    MeshData outData;
    std::ifstream fin(filename);
    if (!fin) throw std::runtime_error("Impossible d'ouvrir le fichier Obj : " + filename);

    std::vector<DirectX::XMFLOAT3> positions;
    std::vector<DirectX::XMFLOAT3> normals;
    std::vector<DirectX::XMFLOAT2> texCoords;

    std::unordered_map<std::string, size_t> indexMap;

    std::string line;
    while (std::getline(fin, line)) {
        std::istringstream ss(line);
        std::string token;
        ss >> token;

        if (token == "v") { // position 3D
            DirectX::XMFLOAT3 p;
            ss >> p.x >> p.y >> p.z;
            positions.push_back(p);
        }
        else if (token == "vn") { // normale
            DirectX::XMFLOAT3 n;
            ss >> n.x >> n.y >> n.z;
            normals.push_back(n);
        }
		else if (token == "vt") { // coordonnée de texture (UV)
            DirectX::XMFLOAT2 t;
            ss >> t.x >> t.y;
            t.y = 1.0f - t.y; 
            texCoords.push_back(t);
        }
        else if (token == "f") { // Face 
            std::vector<size_t> faceIndices;
            std::string corner;

            while (ss >> corner) {
                auto it = indexMap.find(corner);
                if (it != indexMap.end()) {
                    faceIndices.push_back(it->second);
                }
                else {

                    int iPos = 0, iTex = 0, iNorm = 0;
                    sscanf_s(corner.c_str(), "%d/%d/%d", &iPos, &iTex, &iNorm); // lit les data

                    Vertex vert = {}; 
                    if (iPos > 0) vert.Pos = positions[iPos - 1];
                    if (iTex > 0) vert.TexC = texCoords[iTex - 1];
                    if (iNorm > 0) vert.Normal = normals[iNorm - 1];

                    size_t newIndex = static_cast<size_t>(outData.Vertices.size());
                    outData.Vertices.push_back(vert);
                    indexMap[corner] = newIndex;
                    faceIndices.push_back(newIndex);
                }
            }

            for (size_t i = 1; i + 1 < faceIndices.size(); ++i) {
                outData.Indices.push_back(faceIndices[0]);
                outData.Indices.push_back(faceIndices[i]);
                outData.Indices.push_back(faceIndices[i + 1]);
            }
        }
    }

    return outData;
}