#version 330 core

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;

uniform vec3 uColor;
uniform bool uUseTexture;
uniform sampler2D uTexture;

in vec3 vNormal;
in vec2 vTexCoord;

void main()
{
    vec3 n = normalize(vNormal);
    vec3 baseColor = uUseTexture ? texture(uTexture, vTexCoord).rgb : uColor;

    outColor = vec4(baseColor, 1.0);

    vec3 encodedNormal = n * 0.5 + 0.5;
    outNormal = vec4(encodedNormal, 1.0);
}
