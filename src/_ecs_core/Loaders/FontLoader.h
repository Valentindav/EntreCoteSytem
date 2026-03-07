#pragma once
#include <fstream>
#include <sstream>
#include <string>
#include "Data/UiData.h"

class FontLoader {
public:
    static FontData LoadFont(const std::string& filePath, float texWidth, float texHeight) {
        FontData font;
        font.texWidth = texWidth;
        font.texHeight = texHeight;

        std::ifstream file(filePath);
        std::string line;

        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string tag;
            ss >> tag;

            if (tag == "common") {
                std::string attr;
                while (ss >> attr) {
                    if (attr.find("lineHeight=") == 0)
                        font.lineHeight = std::stof(attr.substr(11));
                }
            }
            else if (tag == "char") {
                Glyph g;
                std::string attr;
                while (ss >> attr) {
                    size_t pos = attr.find("=");
                    if (pos == std::string::npos) continue;

                    std::string key = attr.substr(0, pos);
                    float val = std::stof(attr.substr(pos + 1));

                    if (key == "id") g.id = (int)val;
                    else if (key == "x") g.x = val;
                    else if (key == "y") g.y = val;
                    else if (key == "width") g.width = val;
                    else if (key == "height") g.height = val;
                    else if (key == "xoffset") g.xOffset = val;
                    else if (key == "yoffset") g.yOffset = val;
                    else if (key == "xadvance") g.xAdvance = val;
                }

                g.u0 = g.x / texWidth;
                g.v0 = g.y / texHeight;
                g.u1 = (g.x + g.width) / texWidth;
                g.v1 = (g.y + g.height) / texHeight;

                font.glyphs[g.id] = g;
            }
        }
        return font;
    }
};