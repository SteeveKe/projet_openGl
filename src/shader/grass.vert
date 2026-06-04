#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;

uniform mat4 uMvp;
uniform mat4 uModel;
uniform float uTime;

out vec3 vNormal;
out vec2 vTexCoord;

void main()
{
    float t = texCoord.y;

    vec3 pos = position;

    float wave = sin(uTime * 2.0 + position.x * 1.5 + position.z * 1.2);
    pos.x += wave * 0.03 * t * t;

    gl_Position = uMvp * vec4(pos, 1.0);
    vNormal = normalize(mat3(uModel) * normal);
    vTexCoord = texCoord; // uv
}
