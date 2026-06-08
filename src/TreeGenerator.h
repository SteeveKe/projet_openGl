#pragma once

#include "Mesh.h"
#include "MathUtils.h"

#include <filesystem>
#include <vector>

struct TreeField {
    Mesh mesh;
    std::vector<Mat4> transforms;
};

TreeField generateTrees(int count, float spread, const std::filesystem::path& objPath);
