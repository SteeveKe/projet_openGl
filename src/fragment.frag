#version 330 core

out vec4 color;

uniform vec3 uColor;
uniform bool uUseTexture;
uniform sampler2D uTexture;

in vec3 vNormal;
in vec2 vTexCoord;

void main()
{
    vec3 lightDirection = normalize(vec3(0.4, 0.8, 0.6));
    float light = max(dot(normalize(vNormal), lightDirection), 0.0);
    vec3 baseColor = uUseTexture ? texture(uTexture, vTexCoord).rgb : uColor;
    color = vec4(baseColor * (0.25 + light * 0.75), 1.0);
}
