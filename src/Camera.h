#pragma once

#include "MathUtils.h"

struct Camera {
    float yaw    =  0.0f;
    float pitch  =  0.3f;
    float radius =  1.6f;
    float targetY = -0.7f;

    float speed = 0.05f;

    void rotateLeft()  { yaw   += speed; }
    void rotateRight() { yaw   -= speed; }
    void rotateUp()    { pitch += speed; if (pitch >  1.4f) pitch =  1.4f; }
    void rotateDown()  { pitch -= speed; if (pitch < -1.4f) pitch = -1.4f; }
    void zoomIn()      { radius -= 0.3f; if (radius < 0.5f)  radius = 0.5f;  }
    void zoomOut()     { radius += 0.3f; if (radius > 20.0f) radius = 20.0f; }

    Mat4 viewMatrix() const;
};
