#pragma once

#include <GL/glew.h>

#include <array>

struct Mesh {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint texture = 0;
    GLsizei vertexCount = 0;
    std::array<float, 3> color = {0.78f, 0.62f, 0.38f};
    bool hasTexture = false;
};
