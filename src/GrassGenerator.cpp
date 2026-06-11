#include "GrassGenerator.h"
#include "Texture.h"

#include <GL/glew.h>

#include <cmath>
#include <cstdlib>
#include <vector>

#include "TerrainGenerator.h"

void addCard(std::vector<float>& v, float px, float py, float pz, float angle)
{
    const float w = 0.15f;
    const float h = 0.35f;

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


Mesh generateGrass(int count, float spread, const std::filesystem::path& texture_path)
{
    std::vector<float> templateVerts;
    add_grass(templateVerts, 0.0f, 0.0f, 0.0f);
    std::vector<float> instances;
    instances.reserve(count * 4);
    for (int i = 0; i < count; i++) {
        const float px = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * spread;
        const float pz = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * spread;
        const float angle = (float)rand() / RAND_MAX * 6.2831853f;
        instances.push_back(px);
        instances.push_back(terrainHeight(px, pz));
        instances.push_back(pz);
        instances.push_back(angle);
    }

    const GLuint texture = loadTexture(texture_path);

    Mesh mesh;
    mesh.instanceCount = count;

    SubMesh subMesh;
    subMesh.firstVertex = 0;
    subMesh.vertexCount = static_cast<GLsizei>(templateVerts.size() / 8);
    subMesh.texture = texture;
    subMesh.hasTexture = texture != 0;
    mesh.subMeshes.push_back(subMesh);
    mesh.vertexCount = subMesh.vertexCount;

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(templateVerts.size() * sizeof(float)), templateVerts.data(), GL_STATIC_DRAW);
    constexpr GLsizei stride = 8 * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glGenBuffers(1, &mesh.instanceVbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.instanceVbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(instances.size() * sizeof(float)), instances.data(), GL_STATIC_DRAW);
    constexpr GLsizei instanceStride = 4 * sizeof(float);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, instanceStride, nullptr);                              // pos xyz
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, instanceStride, reinterpret_cast<void*>(3 * sizeof(float))); // angle
    glEnableVertexAttribArray(4);
    glVertexAttribDivisor(4, 1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return mesh;
}
