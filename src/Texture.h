#pragma once

#include <GL/glew.h>

#include <filesystem>

GLuint loadTexture(const std::filesystem::path& path);

GLuint generateTexture(int width, int height);
