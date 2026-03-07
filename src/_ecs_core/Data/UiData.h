#pragma once
#include <vector>
#include  <DirectXMath.h>
#include <unordered_map>
#include <string>
using namespace DirectX;
struct UIVertex
{
    XMFLOAT2 Pos;
    XMFLOAT2 UV;
    XMFLOAT4 Color;
    float TextureIndex;
};

struct UIConstants
{
    XMFLOAT2 ScreenSize;
    XMFLOAT2 Padding; // alignement 16 bytes
};

struct DDS_PIXELFORMAT
{
    uint32_t size;
    uint32_t flags;
    uint32_t fourCC;
    uint32_t RGBBitCount;
    uint32_t RBitMask;
    uint32_t GBitMask;
    uint32_t BBitMask;
    uint32_t ABitMask;
};

struct DDS_HEADER
{
    uint32_t size;
    uint32_t flags;
    uint32_t height;
    uint32_t width;
    uint32_t pitchOrLinearSize;
    uint32_t depth;
    uint32_t mipMapCount;
    uint32_t reserved1[11];
    DDS_PIXELFORMAT ddspf;
    uint32_t caps;
    uint32_t caps2;
    uint32_t caps3;
    uint32_t caps4;
    uint32_t reserved2;
};



struct Glyph {
    int id;
    float x, y, width, height;
    float xOffset, yOffset;
    float xAdvance;
    float u0, v0, u1, v1; // Coordonnées UV calculées pour le shader
};

struct FontData {
    std::wstring fontName;
    float size;
    float lineHeight;
    float texWidth, texHeight;
    std::unordered_map<int, Glyph> glyphs;
};