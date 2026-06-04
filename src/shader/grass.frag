#version 330 core

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;

uniform vec3 uColor;
uniform bool uUseTexture;
uniform sampler2D uTexture;
uniform vec3 uLightDir;

in vec3 vNormal;
in vec2 vTexCoord;

void main()
{

    // color grass

    vec4 texColor = texture(uTexture, vTexCoord);
    if (texColor.a < 0.9) // transparence
        discard;
    float t = vTexCoord.y;
    vec3 colorBase = vec3(0.05, 0.25, 0.03);
    vec3 colorTip  = vec3(0.55, 0.85, 0.15);
    vec3 grassColor = mix(colorBase, colorTip, t * t) * texColor.rgb * 2.0;
    outColor = vec4(grassColor, 1.0);

    //light
    vec3 lightDir = normalize(uLightDir);
    float backLight = max(dot(-vNormal, lightDir), 0.0);
    float subsurface = pow(backLight, 2.0) * 0.6;
    vec3 yellow = vec3(0.6, 0.9, 0.2); 
    grassColor += yellow * subsurface * t;


    // normal
    vec3 n = normalize(vNormal);
    outNormal = vec4(0.5, 1.0, 0.5, 1.0);

}
//outColor = vec4(texColor.rgb, 1.0);
//outNormal = vec4(n * 0.5 + 0.5, 1.0);
