#version 330 core

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;

uniform vec3 uColor;
uniform bool uUseTexture;
uniform sampler2D uTexture;

in vec3 vNormal;
in vec2 vTexCoord;
in float vHeight;
in float vDist;

float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    return mix(
        mix(hash(i), hash(i + vec2(1,0)), f.x),
        mix(hash(i + vec2(0,1)), hash(i + vec2(1,1)), f.x),
        f.y
    );
}

vec3 terrainColor()
{
    float slope = 1.0 - vNormal.y;
    float n1 = noise(vTexCoord * 5.0);
    float n2 = noise(vTexCoord * 0.60);
    float n3 = noise(vTexCoord * 0.2);

    vec3 colorGrass1 = vec3(1.0, 0.5, 0.0);
    vec3 colorGrass2 = vec3(0.25, 0.58, 0.10);
    vec3 colorDirt1  = vec3(0.35, 0.28, 0.15);
    vec3 colorDirt2  = vec3(0.48, 0.38, 0.22);
    vec3 colorRock1  = vec3(0.42, 0.40, 0.38);
    vec3 colorRock2  = vec3(0.52, 0.48, 0.43);
    vec3 colorFog = vec3(0.18, 0.20, 0.25);

    vec3 grassColor = mix(colorGrass1, colorGrass2, 0.4 + n1 * 0.6);
    vec3 dirtColor  = mix(colorDirt1, colorDirt2, n2);
    vec3 rockColor  = mix(colorRock1, colorRock2, n1);

    vec3 color = grassColor;
    color = mix(color, dirtColor, smoothstep(0.0, 0.35, slope));
    color = mix(color, rockColor, smoothstep(0.25, 0.55, slope));
    color = mix(color, dirtColor, smoothstep(0.4, 0.9, vHeight));
    color = mix(color, colorFog, smoothstep(15.0, 70.0, vDist));

    color *= 0.88 + 0.24 * n3;
    return color;
}

void main()
{
    vec3 n = normalize(vNormal);
//<<<<<<< HEAD
//    vec3 baseColor = uUseTexture ? texture(uTexture, vTexCoord).rgb : uColor;
//
//    //Attachment 0 couleur brute
//    outColor = vec4(baseColor, 1.0);
//
//    //Attachment 1 normale encodee en [0, 1]
//    outNormal = vec4(n * 0.5 + 0.5, 1.0);
//=======
    vec3 baseColor = uUseTexture ? texture(uTexture, vTexCoord).rgb : terrainColor();
    outColor = vec4(baseColor, 1.0);

    vec3 encodedNormal = n * 0.5 + 0.5;
    outNormal = vec4(encodedNormal, 1.0);

}
