#include "GrassGenerator.h"
#include "Texture.h"

#include <GL/glew.h>

#include <cmath>
#include <cstdlib>
#include <vector>

#include "PerlinNoise.h"

void addCard(std::vector<float>& v, float px, float py, float pz, float angle)
{
    const float w = 0.15f;
    const float h = 0.45f;

    const float dx = std::cos(angle) * w;
    const float dz = std::sin(angle) * w;
    const float nx = -std::sin(angle);
    const float nz =  std::cos(angle);

    v.insert(v.end(), {px - dx, py,     pz - dz,  nx, 0, nz,  0, 0});
    v.insert(v.end(), {px + dx, py,     pz + dz,  nx, 0, nz,  1, 0});
    v.insert(v.end(), {px + dx, py + h, pz + dz,  nx, 0, nz,  1, 1});

    v.insert(v.end(), {px - dx, py,     pz - dz,  nx, 0, nz,  0, 0});
    v.insert(v.end(), {px + dx, py + h, pz + dz,  nx, 0, nz,  1, 1});
    v.insert(v.end(), {px - dx, py + h, pz - dz,  nx, 0, nz,  0, 1});
}

void add_grass(std::vector<float>& v, float px, float py, float pz)
{
    addCard(v, px, py, pz,  0.0f);
    addCard(v, px, py, pz,  3.14159265f / 3.0f);
    addCard(v, px, py, pz,  2.0f * 3.14159265f / 3.0f);
}

float terrain_height(float px, float pz)
{
    return perlinNoise(px * 0.2f, pz * 0.2f) * 0.4f - 0.9f;
    //float h = 0.2f;
    //h += perlinNoise(px * 0.08f, pz * 0.08f) * 1.2f;  // grandes collines
    //h += perlinNoise(px * 0.25f, pz * 0.25f) * 0.3f;  // ondulations moyennes
    //h += perlinNoise(px * 0.7f,  pz * 0.7f)  * 0.08f; // micro détails
    //return h - 1.3f;
}

Mesh generateGrass(int count, float spread, const std::filesystem::path& texture_path)
{
    std::vector<float> vertices;
    //const float groundY = -0.9f;

    for (int i = 0; i < count; i++) {
        const float px = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * spread;
        const float pz = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * spread;
        add_grass(vertices, px, terrain_height(px, pz), pz);
    }

    const GLuint texture = loadTexture(texture_path);

    Mesh mesh;

    SubMesh subMesh;
    subMesh.firstVertex = 0;
    subMesh.vertexCount = static_cast<GLsizei>(vertices.size() / 8);
    subMesh.texture = texture;
    subMesh.hasTexture = texture != 0;
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
