#include "ObjLoader.h"

#include "Texture.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <vector>

Mesh loadObjModel(const std::filesystem::path& objPath)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string message;

    const std::filesystem::path modelDirectory = objPath.parent_path();
    const std::string materialDirectory = modelDirectory.string() + "/";
    const bool loaded = tinyobj::LoadObj(
        &attrib,
        &shapes,
        &materials,
        &message,
        objPath.string().c_str(),
        materialDirectory.c_str(),
        true
    );

    if (!message.empty()) {
        std::cerr << message << std::endl;
    }

    if (!loaded) {
        std::cerr << "Impossible de charger le modele : " << objPath << std::endl;
        return {};
    }

    if (attrib.vertices.empty()) {
        std::cerr << "Le modele ne contient aucun sommet : " << objPath << std::endl;
        return {};
    }

    std::array<float, 3> minPosition = {
        attrib.vertices[0],
        attrib.vertices[1],
        attrib.vertices[2],
    };
    std::array<float, 3> maxPosition = minPosition;

    for (std::size_t index = 0; index < attrib.vertices.size(); index += 3) {
        for (int axis = 0; axis < 3; ++axis) {
            const float value = attrib.vertices[index + axis];
            minPosition[axis] = std::min(minPosition[axis], value);
            maxPosition[axis] = std::max(maxPosition[axis], value);
        }
    }

    const std::array<float, 3> center = {
        (minPosition[0] + maxPosition[0]) * 0.5f,
        (minPosition[1] + maxPosition[1]) * 0.5f,
        (minPosition[2] + maxPosition[2]) * 0.5f,
    };
    const float sizeX = maxPosition[0] - minPosition[0];
    const float sizeY = maxPosition[1] - minPosition[1];
    const float sizeZ = maxPosition[2] - minPosition[2];
    const float scale = 1.8f / std::max(sizeX, std::max(sizeY, sizeZ));

    std::vector<float> vertices;

    for (const tinyobj::shape_t& shape : shapes) {
        std::size_t indexOffset = 0;
        for (std::size_t faceIndex = 0; faceIndex < shape.mesh.num_face_vertices.size(); ++faceIndex) {
            const int faceVertexCount = shape.mesh.num_face_vertices[faceIndex];

            for (int vertexIndex = 0; vertexIndex < faceVertexCount; ++vertexIndex) {
                const tinyobj::index_t objIndex = shape.mesh.indices[indexOffset + vertexIndex];
                const int positionIndex = 3 * objIndex.vertex_index;
                const int normalIndex = 3 * objIndex.normal_index;
                const int texCoordIndex = 2 * objIndex.texcoord_index;

                vertices.push_back((attrib.vertices[positionIndex] - center[0]) * scale);
                vertices.push_back((attrib.vertices[positionIndex + 1] - center[1]) * scale);
                vertices.push_back((attrib.vertices[positionIndex + 2] - center[2]) * scale);

                if (objIndex.normal_index >= 0) {
                    vertices.push_back(attrib.normals[normalIndex]);
                    vertices.push_back(attrib.normals[normalIndex + 1]);
                    vertices.push_back(attrib.normals[normalIndex + 2]);
                } else {
                    vertices.push_back(0.0f);
                    vertices.push_back(0.0f);
                    vertices.push_back(1.0f);
                }

                if (objIndex.texcoord_index >= 0) {
                    vertices.push_back(attrib.texcoords[texCoordIndex]);
                    vertices.push_back(attrib.texcoords[texCoordIndex + 1]);
                } else {
                    vertices.push_back(0.0f);
                    vertices.push_back(0.0f);
                }
            }

            indexOffset += faceVertexCount;
        }
    }

    Mesh mesh;
    mesh.vertexCount = static_cast<GLsizei>(vertices.size() / 8);

    for (const tinyobj::material_t& material : materials) {
        if (!material.diffuse_texname.empty()) {
            mesh.texture = loadTexture(modelDirectory / material.diffuse_texname);
            mesh.hasTexture = mesh.texture != 0;
            break;
        }
    }

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

    std::cout << "Modele charge : " << mesh.vertexCount << " sommets" << std::endl;
    return mesh;
}
