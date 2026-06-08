#include "RockGenerator.h"
#include "ObjLoader.h"
#include "TerrainGenerator.h"

#include <cstdlib>

RockField generateRocks(int count, float spread, const std::filesystem::path& objPath, float scaleMin, float scaleMax)
{
    RockField field;
    field.mesh = loadObjModel(objPath);

    for (int i = 0; i < count; i++) {
        float px    = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * spread;
        float pz    = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * spread;
        float angle = (float)rand() / RAND_MAX * 6.28318f;
        float s     = scaleMin + (float)rand() / RAND_MAX * (scaleMax - scaleMin);

        Mat4 t = multiply(translate(px, terrainHeight(px, pz), pz),
                  multiply(rotateY(angle), scale(s)));
        field.transforms.push_back(t);
    }

    return field;
}
