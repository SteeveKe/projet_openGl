#include "Camera.h"
#include <cmath>

Mat4 Camera::viewMatrix() const
{
    float cp = cosf(pitch), sp = sinf(pitch);
    float cy = cosf(yaw),   sy = sinf(yaw);

    // right = ( cy,       0,  -sy      )
    // up    = (-sp*sy,   cp,  -sp*cy   )
    // fwd   = ( cp*sy,   sp,   cp*cy   )  (pointe de target vers eye)

    Mat4 m{};
    // colonne 0
    m[0] =  cy;     m[1] = -sp*sy;  m[2] =  cp*sy;  m[3] = 0.0f;
    // colonne 1
    m[4] =  0.0f;   m[5] =  cp;     m[6] =  sp;     m[7] = 0.0f;
    // colonne 2
    m[8] = -sy;     m[9] = -sp*cy;  m[10]=  cp*cy;  m[11]= 0.0f;
    // colonne 3 : translation
    m[12]=  0.0f;
    m[13]= -targetY * cp;
    m[14]= -(radius + targetY * sp);
    m[15]=  1.0f;
    return m;
}
