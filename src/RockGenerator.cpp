#include "RockGenerator.h"
#include "ObjLoader.h"
#include "TerrainGenerator.h"

#include <cstdlib>
#include <cmath>

RockField generateRocks(int count, float spread, const std::filesystem::path& objPath)
{
    RockField field;
    field.mesh = loadObjModel(objPath);

    for (int i = 0; i < count; i++) {
        float px    = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * spread;
        float pz    = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * spread;
        float angle = (float)rand() / RAND_MAX * 6.28318f;
        float dist       = sqrtf(px * px + pz * pz);
        float sizeReduce = 1.0f - 0.65f * (dist / spread);
        sizeReduce       = sizeReduce < 0.3f ? 0.3f : sizeReduce;
        float s          = (0.15f + (float)rand() / RAND_MAX * 0.2f) * sizeReduce;

        Mat4 t = multiply(translate(px, terrainHeight(px, pz), pz), multiply(rotateY(angle), scale(s)));
        field.transforms.push_back(t);
    }

    return field;
}
