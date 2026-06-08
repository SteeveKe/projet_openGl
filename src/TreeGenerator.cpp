#include "TreeGenerator.h"
#include "ObjLoader.h"
#include "TerrainGenerator.h"

#include <cstdlib>

TreeField generateTrees(int count, float spread, const std::filesystem::path& objPath)
{
    TreeField field;
    field.mesh = loadObjModel(objPath);

    for (int i = 0; i < count; i++) {
        float px    = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * spread;
        float pz    = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * spread;
        float angle = (float)rand() / RAND_MAX * 6.28318f;
        float s     = 0.7f + (float)rand() / RAND_MAX * 0.1f;

        Mat4 t = multiply(translate(px, terrainHeight(px, pz) + 1.1f * s, pz), multiply(rotateY(angle), scale(s)));
        field.transforms.push_back(t);
    }

    return field;
}
