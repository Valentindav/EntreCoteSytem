#pragma once
#include <string>
#include "Data/MeshData.h"

class LoaderObj {
public:
   static MeshData Load(const std::string& filename);
};