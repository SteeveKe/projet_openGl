#pragma once

#include "MathUtils.h"

struct Camera {
    float yaw   =  0.0f;   // rotation horizontale (gauche/droite)
    float pitch = -0.28f;  // rotation verticale (légèrement vers le bas)
    float x = 0.0f;
    float y = -0.2f;       // proche du sol
    float z = 1.5f;        // proche du personnage

    float speed = 0.05f;

    void rotateLeft()  { yaw   += speed; }
    void rotateRight() { yaw   -= speed; }
    void rotateUp()    { pitch += speed; if (pitch >  1.4f) pitch =  1.4f; }
    void rotateDown()  { pitch -= speed; if (pitch < -1.4f) pitch = -1.4f; }

    Mat4 viewMatrix() const;
};
