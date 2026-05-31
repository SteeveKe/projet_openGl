#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;

uniform mat4 uMvp;
uniform mat4 uModel;

out vec3 vNormal;
out vec2 vTexCoord;


void main()
{
    gl_Position = uMvp * vec4(position, 1.0);

    vNormal = normalize(mat3(uModel) * normal);
    vTexCoord = texCoord;
    //vColor = normalize(vNormal) * 0.5 + 0.5;
}
