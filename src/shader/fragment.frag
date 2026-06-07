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

    //Attachment 0 couleur brute
    outColor = vec4(baseColor, 1.0);

    //Attachment 1 normale encodee en [0, 1]
    outNormal = vec4(n * 0.5 + 0.5, 1.0);

}
