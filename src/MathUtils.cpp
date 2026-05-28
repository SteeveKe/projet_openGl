#include "MathUtils.h"

#include <cmath>

Mat4 identity()
{
    return {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
}

Mat4 multiply(const Mat4& left, const Mat4& right)
{
    Mat4 result{};

    for (int column = 0; column < 4; ++column) {
        for (int row = 0; row < 4; ++row) {
            float value = 0.0f;
            for (int index = 0; index < 4; ++index) {
                value += left[index * 4 + row] * right[column * 4 + index];
            }
            result[column * 4 + row] = value;
        }
    }

    return result;
}

Mat4 translate(float x, float y, float z)
{
    Mat4 result = identity();
    result[12] = x;
    result[13] = y;
    result[14] = z;
    return result;
}

Mat4 rotateX(float angle)
{
    Mat4 result = identity();
    const float c = std::cos(angle);
    const float s = std::sin(angle);

    result[5] = c;
    result[6] = s;
    result[9] = -s;
    result[10] = c;

    return result;
}

Mat4 rotateY(float angle)
{
    Mat4 result = identity();
    const float c = std::cos(angle);
    const float s = std::sin(angle);

    result[0] = c;
    result[2] = -s;
    result[8] = s;
    result[10] = c;

    return result;
}

Mat4 perspective(float fovRadians, float aspect, float nearPlane, float farPlane)
{
    const float f = 1.0f / std::tan(fovRadians * 0.5f);

    Mat4 result{};
    result[0] = f / aspect;
    result[5] = f;
    result[10] = (farPlane + nearPlane) / (nearPlane - farPlane);
    result[11] = -1.0f;
    result[14] = (2.0f * farPlane * nearPlane) / (nearPlane - farPlane);

    return result;
}
