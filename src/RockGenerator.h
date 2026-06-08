#pragma once

#include "Mesh.h"
#include "MathUtils.h"

#include <filesystem>
#include <vector>

struct RockField {
    Mesh mesh;
    std::vector<Mat4> transforms;
};

RockField generateRocks(int count, float spread, const std::filesystem::path& objPath, float scaleMin = 0.15f, float scaleMax = 0.35f);
