#pragma once

#include <GL/glew.h>

#include <filesystem>

GLuint createShaderProgram(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath);
