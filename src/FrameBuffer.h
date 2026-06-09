#pragma once
#include <GL/glew.h>

void generateFrameBuffer(
    int width,
    int height,
    GLuint& colorTexture,
    GLuint& normalTexture,
    GLuint& sobelMaskTexture,
    GLuint& depthTexture,
    GLuint& framebuffer
);

void resizeFrameBuffer(
    int width,
    int height,
    GLuint colorTexture,
    GLuint normalTexture,
    GLuint sobelMaskTexture,
    GLuint depthTexture,
    GLuint framebuffer
);

//GLuint generateDepthTexture(int width, int height);
