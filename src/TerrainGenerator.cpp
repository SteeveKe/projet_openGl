#include "TerrainGenerator.h"
#include "PerlinNoise.h"

#include <GL/glew.h>

#include <vector>

float terrainHeight(float px, float pz)
{
    return perlinNoise(px * 0.5f, pz * 0.5f) * 0.4f - 0.9f;
    //float h = 0.2f;
    //h += perlinNoise(px * 0.08f, pz * 0.08f) * 1.2f;  // grandes collines
    //h += perlinNoise(px * 0.25f, pz * 0.25f) * 0.3f;  // ondulations moyennes
    //h += perlinNoise(px * 0.7f,  pz * 0.7f)  * 0.08f; // micro détails
    //return h - 1.3f;
}

// on l'approxime en regardant les hauteurs des voisins gauche/droite/haut/bas
static std::array<float, 3> computeNormal(float x, float z, float step)
{
    float hL = terrainHeight(x - step, z);  
    float hR = terrainHeight(x + step, z);  
    float hD = terrainHeight(x, z - step);  
    float hU = terrainHeight(x, z + step);
    float nx = hL - hR;
    float ny = 2.0f * step;
    float nz = hD - hU;
    float len = std::sqrt(nx*nx + ny*ny + nz*nz);
    return {nx / len, ny / len, nz / len};
}

static void push_vertex(std::vector<float> &vertices, float x, float z, float y, float step, float res, float i, float j)
{
    vertices.push_back(x);
    vertices.push_back(y);
    vertices.push_back(z);

    auto n = computeNormal(x, z, step);
    vertices.push_back(n[0]);
    vertices.push_back(n[1]);
    vertices.push_back(n[2]);

    vertices.push_back((float)i / res);
    vertices.push_back((float)j / res);
}

static void push_triangles(std::vector<float> &vertices, float i, float j, float step, float resolution)
{
    float half = resolution * step / 2;
    float x0 = i * step - half;
    float z0 = j * step - half;
    float x1 = x0 + step;
    float z1 = z0 + step;

    float y00 = terrainHeight(x0, z0);
    float y01 = terrainHeight(x0, z1);
    float y10 = terrainHeight(x1, z0);
    float y11 = terrainHeight(x1, z1);

    // triangle 1
    push_vertex(vertices, x0, z0, y00, step, resolution, i, j);
    push_vertex(vertices, x1, z1, y11, step, resolution, i, j);
    push_vertex(vertices, x0, z1, y01, step, resolution, i, j);

    // triangle 2
    push_vertex(vertices, x0, z0, y00, step, resolution, i, j);
    push_vertex(vertices, x1, z0, y10, step, resolution, i, j);
    push_vertex(vertices, x1, z1, y11, step, resolution, i, j);
}

Mesh generateTerrain(int resolution, float size)
{
    std::vector<float> vertices;

    const float step = size / resolution;  // distance entre deux vertices

    for (size_t i = 0; i < resolution; i+=1)
    {
        for (size_t j = 0; j < resolution; j+=1)
        {
            push_triangles(vertices, i, j , step, resolution); 
        }
    }

    SubMesh subMesh;
    subMesh.firstVertex = 0;
    subMesh.vertexCount = static_cast<GLsizei>(vertices.size() / 8);
    //subMesh.color = {0.35f, 0.28f, 0.15f};  // marron
    subMesh.color = {0.13f, 0.45f, 0.08f};  // vert zelda

    subMesh.hasTexture = false;

    Mesh mesh;
    mesh.subMeshes.push_back(subMesh);
    mesh.vertexCount = subMesh.vertexCount;

    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);
    glBindVertexArray(mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(float)), vertices.data(), GL_STATIC_DRAW);
    constexpr GLsizei stride = 8 * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return mesh;
}
