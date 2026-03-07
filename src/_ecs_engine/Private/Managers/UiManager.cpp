#include "UiManager.h"
#include <iostream>

void UiManager::Initialize(ID3D12Device* device, int maxVertices)
{
    uiVertexBuffer = std::make_unique<UploadBuffer<UIVertex>>(device, maxVertices, false);
}

void UiManager::AddUIImage(float x, float y, float width, float height, int textureIndex,
    float u0, float v0, float u1, float v1)
{
    float left = x - width / 2;
    float right = x + width / 2;
    float top = y - height / 2;
    float bottom = y + height / 2;

    XMFLOAT4 color = { 1, 1, 1, 1 };
    float texIdx = (float)textureIndex;

    uiVerticesCPU.push_back({ {left,  top},    {u0, v0}, color, texIdx });
    uiVerticesCPU.push_back({ {right, top},    {u1, v0}, color, texIdx });
    uiVerticesCPU.push_back({ {left,  bottom}, {u0, v1}, color, texIdx });

    uiVerticesCPU.push_back({ {right, top},    {u1, v0}, color, texIdx });
    uiVerticesCPU.push_back({ {right, bottom}, {u1, v1}, color, texIdx });
    uiVerticesCPU.push_back({ {left,  bottom}, {u0, v1}, color, texIdx });
}


void UiManager::AddUIText(std::string text, float x, float y, const FontData& font, int textureIndex) {
    float curX = x;
    float curY = y;
    float texIdx = (float)textureIndex;

    for (char c : text) {
        if (font.glyphs.count(c)) {
            const Glyph& g = font.glyphs.at(c);

            float left = curX + g.xOffset;
            float top = curY + g.yOffset;
            float right = left + g.width;
            float bottom = top + g.height;

            XMFLOAT4 white = { 1.0f, 1.0f, 1.0f, 1.0f };

            uiVerticesCPU.push_back({ {left, top}, {g.u0, g.v0}, white, texIdx });
            uiVerticesCPU.push_back({ {right, top},    {g.u1, g.v0}, white,texIdx });
            uiVerticesCPU.push_back({ {left,  bottom}, {g.u0, g.v1}, white,texIdx });

            uiVerticesCPU.push_back({ {right, top},    {g.u1, g.v0}, white,texIdx });
            uiVerticesCPU.push_back({ {right, bottom}, {g.u1, g.v1}, white,texIdx });
            uiVerticesCPU.push_back({ {left,  bottom}, {g.u0, g.v1}, white,texIdx });

            curX += g.xAdvance;
        }
    }
}

int UiManager::AddTextureToLists(ComPtr<ID3D12Resource>& Texture)
{

    for (int i = 0; i < uiTexturelist.size(); ++i) {
        if (uiTexturelist[i].Get() == Texture.Get()) {
            return i;
        }
    }


    uiTexturelist.push_back(Texture);
    int newIndex = (int)uiTexturelist.size() - 1;
    textureCount = (int)uiTexturelist.size();
    return newIndex;
}

void UiManager::ClearUI() {
    uiVerticesCPU.clear();
}
