#pragma once
#include <string>

class Entity;

namespace Ui
{
	Entity* Image(float x, float y, float w, float h, std::string filePath);
	Entity* Text(float x, float y, std::string filePath);
	Entity* Button(float x, float y, float w, float h, std::string normalFilePath, std::string hoverFilePath);
}