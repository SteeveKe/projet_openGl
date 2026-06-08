#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;
layout (location = 3) in vec3  aInstancePos;
layout (location = 4) in float aInstanceAngle;

uniform mat4 uMvp;
uniform mat4 uModel;
uniform float uTime;

out vec3 vNormal;
out vec2 vTexCoord;
out float vDist;
out vec2 vWorldXZ;

float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    return mix( mix(hash(i), hash(i + vec2(1,0)), f.x), mix(hash(i + vec2(0,1)), hash(i + vec2(1,1)), f.x), f.y);
}

void main()
{
    float t = texCoord.y;

    float holeScale = noise(aInstancePos.xz * 0.9);
    float heightScale = (0.1 + hash(aInstancePos.xz) * 0.76);
    if (holeScale < 0.4) 
        heightScale = (0.1 + hash(aInstancePos.xz) * 0.5);
    if (holeScale < 0.3) 
        heightScale = (0.1 + hash(aInstancePos.xz) * 0.2);

    // rotation Y par l'angle de l'instance
    float c = cos(aInstanceAngle);
    float s = sin(aInstanceAngle);
    vec3 pos = vec3(
        position.x * c - position.z * s,
        position.y * heightScale,
        position.x * s + position.z * c
    );

    // vent basé sur la position monde de l'instance
    float wave = sin(uTime * 2.0 + aInstancePos.x * 1.5 + aInstancePos.z * 1.2);
    pos.x += wave * 0.01 * t * t;

    // translation vers la position monde
    pos += aInstancePos;

    gl_Position = uMvp * vec4(pos, 1.0);
    vNormal = normalize(mat3(uModel) * normal);
    vTexCoord = texCoord;
    vDist = length(aInstancePos.xz);
    vWorldXZ = aInstancePos.xz;
}
