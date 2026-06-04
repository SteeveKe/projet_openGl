#pragma once

#include "Mesh.h"

#include <filesystem>

Mesh generateGrass(int count, float spread, const std::filesystem::path& texturePath);
