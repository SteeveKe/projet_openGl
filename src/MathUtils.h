#pragma once

#include <array>

using Mat4 = std::array<float, 16>;

Mat4 identity();
Mat4 multiply(const Mat4& left, const Mat4& right);
Mat4 translate(float x, float y, float z);
Mat4 rotateX(float angle);
Mat4 rotateY(float angle);
Mat4 perspective(float fovRadians, float aspect, float nearPlane, float farPlane);
