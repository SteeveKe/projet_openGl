#pragma once

#include "Mesh.h"

#include <filesystem>

Mesh loadObjModel(const std::filesystem::path& objPath);
