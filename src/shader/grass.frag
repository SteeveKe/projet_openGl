#version 330 core

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out float outSobelMask;

uniform vec3 uColor;
uniform bool uUseTexture;
uniform sampler2D uTexture;
uniform vec3 uLightDir;

in vec3 vNormal;
in vec2 vTexCoord;
in float vDist;
in vec2 vWorldXZ;

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

void main()
{
    //if (vDist > 10.0)
    //    discard;
    float fadeStart = 3.0;
    float fadeEnd   = 10.0;
    float fadeFactor = smoothstep(fadeStart, fadeEnd, vDist);
    if (fadeFactor > hash(vWorldXZ * 7.3 + vec2(0.5))) discard;



    // color grass
    vec4 texColor = texture(uTexture, vTexCoord);
    if (texColor.a < 0.65) // transparence
        discard;
    float t = vTexCoord.y;
    // 1
    //vec3 colorBase = vec3(0.12, 0.45, 0.05);
    //vec3 colorTip  = vec3(0.55, 1.10, 0.40);
    // 2

    //vec3 colorBase = vec3(0.08, 0.35, 0.04);
    //vec3 colorTip  = vec3(0.32, 0.72, 0.12);

    //vec3 colorBase = vec3(0.12, 0.42, 0.06);
    vec3 colorBase = vec3(0.08, 0.35, 0.04);
    vec3 colorTip  = vec3(0.42, 0.85, 0.18);

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
    outNormal = vec4(0.5, 1.0, 0.5, 0.0); // alpha=0 -> detecte comme herbe dans screen.frag
    outSobelMask = 0.3;
    //outNormal = vec4(0.5, 1.0, 0.5, 1.0);
    //outSobelMask = 0.3;
}
//outColor = vec4(texColor.rgb, 1.0);
//outNormal = vec4(n * 0.5 + 0.5, 1.0);
