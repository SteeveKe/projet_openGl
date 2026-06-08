#include "Camera.h"

Mat4 Camera::viewMatrix() const
{
    // vue = rotateX(-pitch) * rotateY(-yaw) * translate(-pos)
    Mat4 t  = translate(-x, -y, -z);
    Mat4 ry = rotateY(-yaw);
    Mat4 rx = rotateX(-pitch);
    return multiply(rx, multiply(ry, t));
}
