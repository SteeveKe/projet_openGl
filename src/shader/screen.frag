#version 330 core

in vec2 vUV;
out vec4 fsColor;

uniform sampler2D colorTexture;
uniform sampler2D normalTexture;
uniform sampler2D depthTexture;
uniform sampler2D sobelMaskTexture;

uniform int debugMode;
uniform vec3 uSunDirection;
uniform vec3 uSunColor;
uniform float uSunIntensity;
uniform float uAmbientLight;
uniform vec2 uLightPosition;
uniform float uLightHeight;

const float nearPlane = 0.1;
const float farPlane = 100.0;
const float DEBUG_DEPTH_NEAR = 2.5;
const float DEBUG_DEPTH_FAR = 3.5;


//ombres en cercles
const float CIRCLE_SHADOW_GRID_STEP = 7.0;
const float CIRCLE_SHADOW_ROTATION = 20.0;
const float CIRCLE_SHADOW_RADIUS = 0.8;
const float CIRCLE_SHADOW_DARKNESS = 0.3;
const float CIRCLE_SHADOW_MIN_LIGHT = 0.05;

//Intensites du cel shading
const float CEL_SHADOW_LIGHT = 0.50;
const float CEL_FULL_LIGHT = 1.00;

//Seuils du two-step cel
const float CEL_STEP_1 = 0.33;
const float CEL_STEP_2 = 0.66;

vec3 decodeNormal(vec3 encodedNormal)
{
    //Pour recuperer normale entre [-1, 1]
    return normalize(encodedNormal * 2.0 - 1.0);
}

float luminance(vec3 color)
{
    return dot(color, vec3(0.299, 0.587, 0.114));
}

float objectMask(vec2 uv)
{
    float rawDepth = texture(depthTexture, uv).r;
    return 1.0 - step(0.9999, rawDepth);
}

float linearDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * nearPlane * farPlane) /
    (farPlane + nearPlane - z * (farPlane - nearPlane));
}

float computeSunDiffuse(vec3 encodedNormal)
{
    vec3 normal = decodeNormal(encodedNormal);
    return max(dot(normal, normalize(uSunDirection)), 0.0);
}

float computeMouseDiffuse(vec2 uv, vec3 encodedNormal)
{
    vec3 normal = decodeNormal(encodedNormal);
    vec2 resolution = vec2(textureSize(colorTexture, 0));
    vec2 lightDelta = uLightPosition - uv;
    lightDelta.x *= resolution.x / resolution.y;

    vec3 lightDirection = normalize(vec3(lightDelta, uLightHeight));
    return max(dot(normal, lightDirection), 0.0);
}

float computeDiffuse(vec2 uv, vec3 encodedNormal)
{
    float sunDiffuse = computeSunDiffuse(encodedNormal);
    float mouseDiffuse = computeMouseDiffuse(uv, encodedNormal);
    return clamp(sunDiffuse * uSunIntensity + mouseDiffuse * 0.65, 0.0, 1.0);
}

vec3 applyCombinedLighting(vec3 color, vec2 uv, vec3 encodedNormal)
{
    float sunDiffuse = computeSunDiffuse(encodedNormal);
    float mouseDiffuse = computeMouseDiffuse(uv, encodedNormal);
    vec3 sunLight = uSunColor * sunDiffuse * uSunIntensity;
    vec3 mouseLight = vec3(1.0, 0.96, 0.88) * mouseDiffuse * 0.65;
    return color * min(vec3(uAmbientLight) + sunLight + mouseLight, vec3(1.6));
}

float twoStepCel(float diffuse)
{
    return 0.5 * (step(CEL_STEP_1, diffuse) + step(CEL_STEP_2, diffuse));
}

float circle(vec2 pixel, vec2 center, float radius)
{
    return 1.0 - smoothstep(radius - 1.0, radius + 1.0, length(pixel - center));
}

mat2 rotate2D(float angle)
{
    float c = cos(angle);
    float s = sin(angle);
    return mat2(c, -s, s, c);
}

// Sobel couleur + normales + profondeur

float sobelFloat(
float topLeft,
float top,
float topRight,
float left,
float right,
float bottomLeft,
float bottom,
float bottomRight
)
{
    float gx = -topLeft + topRight - 2.0 * left + 2.0 * right - bottomLeft + bottomRight;
    float gy = -topLeft - 2.0 * top - topRight + bottomLeft + 2.0 * bottom + bottomRight;
    return abs(gx) + abs(gy);
}

float sobelVec3(
vec3 topLeft,
vec3 top,
vec3 topRight,
vec3 left,
vec3 right,
vec3 bottomLeft,
vec3 bottom,
vec3 bottomRight
)
{
    vec3 gx = -topLeft + topRight - 2.0 * left + 2.0 * right - bottomLeft + bottomRight;
    vec3 gy = -topLeft - 2.0 * top - topRight + bottomLeft + 2.0 * bottom + bottomRight;
    return length(gx) + length(gy);
}

float sobelEdges(vec2 uv)
{
    float outlineSize = 1.0;
    vec2 texel = outlineSize / vec2(textureSize(colorTexture, 0));

    vec2 uvTopLeft     = uv + texel * vec2(-1.0,  1.0);
    vec2 uvTop         = uv + texel * vec2( 0.0,  1.0);
    vec2 uvTopRight    = uv + texel * vec2( 1.0,  1.0);
    vec2 uvLeft        = uv + texel * vec2(-1.0,  0.0);
    vec2 uvRight       = uv + texel * vec2( 1.0,  0.0);
    vec2 uvBottomLeft  = uv + texel * vec2(-1.0, -1.0);
    vec2 uvBottom      = uv + texel * vec2( 0.0, -1.0);
    vec2 uvBottomRight = uv + texel * vec2( 1.0, -1.0);

    //Contours de couleur
    float colorEdge = sobelFloat(
    luminance(texture(colorTexture, uvTopLeft).rgb),
    luminance(texture(colorTexture, uvTop).rgb),
    luminance(texture(colorTexture, uvTopRight).rgb),
    luminance(texture(colorTexture, uvLeft).rgb),
    luminance(texture(colorTexture, uvRight).rgb),
    luminance(texture(colorTexture, uvBottomLeft).rgb),
    luminance(texture(colorTexture, uvBottom).rgb),
    luminance(texture(colorTexture, uvBottomRight).rgb)
    );

    // Contours de normales
    vec3 normalTopLeft     = decodeNormal(texture(normalTexture, uvTopLeft).rgb);
    vec3 normalTop         = decodeNormal(texture(normalTexture, uvTop).rgb);
    vec3 normalTopRight    = decodeNormal(texture(normalTexture, uvTopRight).rgb);
    vec3 normalLeft        = decodeNormal(texture(normalTexture, uvLeft).rgb);
    vec3 normalRight       = decodeNormal(texture(normalTexture, uvRight).rgb);
    vec3 normalBottomLeft  = decodeNormal(texture(normalTexture, uvBottomLeft).rgb);
    vec3 normalBottom      = decodeNormal(texture(normalTexture, uvBottom).rgb);
    vec3 normalBottomRight = decodeNormal(texture(normalTexture, uvBottomRight).rgb);

    float normalEdge = sobelVec3(
    normalTopLeft,
    normalTop,
    normalTopRight,
    normalLeft,
    normalRight,
    normalBottomLeft,
    normalBottom,
    normalBottomRight
    );

    // Contours de profondeur.
    float depthTopLeft     = linearDepth(texture(depthTexture, uvTopLeft).r) / 10.0;
    float depthTop         = linearDepth(texture(depthTexture, uvTop).r) / 10.0;
    float depthTopRight    = linearDepth(texture(depthTexture, uvTopRight).r) / 10.0;
    float depthLeft        = linearDepth(texture(depthTexture, uvLeft).r) / 10.0;
    float depthRight       = linearDepth(texture(depthTexture, uvRight).r) / 10.0;
    float depthBottomLeft  = linearDepth(texture(depthTexture, uvBottomLeft).r) / 10.0;
    float depthBottom      = linearDepth(texture(depthTexture, uvBottom).r) / 10.0;
    float depthBottomRight = linearDepth(texture(depthTexture, uvBottomRight).r) / 10.0;

    float depthEdge = sobelFloat(
    depthTopLeft,
    depthTop,
    depthTopRight,
    depthLeft,
    depthRight,
    depthBottomLeft,
    depthBottom,
    depthBottomRight
    );

    // Reglage du poids des differentes sources de contour
    float edgeStrength = colorEdge * 0.45 + normalEdge * 0.02 + depthEdge * 5.;
    //float edgeStrength = colorEdge * 0.5 + normalEdge * 0.3 + depthEdge * 10.;

    float threshold = 0.18;
    return step(threshold, edgeStrength);
}

float circleShadowMask(float diffuse)
{
    vec2 resolution = vec2(textureSize(colorTexture, 0));
    vec2 pos = gl_FragCoord.xy - 0.5 * resolution;
    pos = rotate2D(radians(CIRCLE_SHADOW_ROTATION)) * pos;

    vec2 gridPos = mod(pos, CIRCLE_SHADOW_GRID_STEP);
    float cel = twoStepCel(diffuse);
    float shadowBand = 1.0 - step(0.999, cel);
    float radius = CIRCLE_SHADOW_RADIUS
    * CIRCLE_SHADOW_GRID_STEP
    * pow(1.0 - diffuse, 2.0);

    return circle(gridPos, vec2(CIRCLE_SHADOW_GRID_STEP * 0.5), radius) * shadowBand;
}


vec3 applyTwoStepCelColor(vec3 baseColor, float diffuse)
{
    float cel = twoStepCel(diffuse);
    float celLighting = mix(CEL_SHADOW_LIGHT, CEL_FULL_LIGHT, cel);

    return baseColor * celLighting;
}

vec3 applyLightingWithCircleShadowsCel(vec3 baseColor, vec3 encodedNormal)
{
    float diffuse = computeDiffuse(vUV, encodedNormal);
    vec3 celColor = applyTwoStepCelColor(baseColor, diffuse);

    float shadowMask = circleShadowMask(diffuse) * objectMask(vUV);
    vec3 shadowColor = max(celColor * CIRCLE_SHADOW_DARKNESS, vec3(CIRCLE_SHADOW_MIN_LIGHT));

    return mix(celColor, shadowColor, shadowMask);
}


vec3 applyLighting(vec3 color, vec3 encodedNormal)
{
    return applyCombinedLighting(color, vUV, encodedNormal);
}

void main()
{
    vec3 baseColor = texture(colorTexture, vUV).rgb;
    vec3 encodedNormal = texture(normalTexture, vUV).rgb;
    float modelMask = objectMask(vUV);

    if (debugMode >= 5 && debugMode <= 9 && modelMask < 0.5) {
        fsColor = vec4(baseColor, 1.0);
        return;
    }

    float diffuse = computeDiffuse(vUV, encodedNormal);
    float cel = twoStepCel(diffuse);
    float sobelIntensity = texture(sobelMaskTexture, vUV).r;
    float edge = sobelEdges(vUV) * sobelIntensity;
    float sunLighting = clamp(uAmbientLight + diffuse, 0.0, 1.0);

    if (debugMode == 0) {
        fsColor = vec4(baseColor, 1.0);
    }
    //normales
    else if (debugMode == 1) {
        fsColor = texture(normalTexture, vUV);
    }
    //profondeur
    else if (debugMode == 2) {
        float depth = texture(depthTexture, vUV).r;
        fsColor = vec4(vec3(depth), 1.0);
    }
    //profondeur linearisee
    else if (debugMode == 3) {
        float depth = texture(depthTexture, vUV).r;
        float visibleDepth = 1.0 - smoothstep(
            DEBUG_DEPTH_NEAR,
            DEBUG_DEPTH_FAR,
            linearDepth(depth)
        );
        fsColor = vec4(vec3(visibleDepth), 1.0);
    }
    //Sobel
    else if (debugMode == 4) {
        fsColor = vec4(uSunColor * edge * sunLighting, 1.0);
    }
    //diffuse
    else if (debugMode == 5) {
        fsColor = vec4(vec3(diffuse), 1.0);
    }
    //two-step cel
    else if (debugMode == 6) {
        fsColor = vec4(vec3(cel), 1.0);
    }
    //couleur + two-step cel
    else if (debugMode == 7) {
        vec3 celColor = applyTwoStepCelColor(baseColor, diffuse);
        fsColor = vec4(celColor, 1.0);
    }
    //ombres en points
    else if (debugMode == 8) {
        float shadowMask = circleShadowMask(diffuse);
        fsColor = vec4(vec3(1.0 - shadowMask), 1.0);
    }
    //rendu final ombres en points + Sobel + couleur + two-step cel
    else if (debugMode == 9) {
        vec3 finalColor = applyLightingWithCircleShadowsCel(baseColor, encodedNormal);
        vec3 outlineColor = baseColor * 0.22;
        fsColor = vec4(mix(finalColor, outlineColor, edge), 1.0);
    }
    //else if (debugMode == 10) {
    //    float rawDepth = texture(depthTexture, vUV).r;
    //    if (rawDepth >= 0.9999) {
    //        vec3 skyTop     = vec3(0.30, 0.52, 0.80);
    //        vec3 skyHorizon = vec3(0.90, 0.95, 0.98);
    //        fsColor = vec4(mix(skyHorizon, skyTop, vUV.y), 1.0);
    //        return;
    //    }
    //    vec3 finalColor = applyLightingWithCircleShadowsCel(baseColor, encodedNormal);
    //    vec3 smoothLighting = applyLighting(baseColor, encodedNormal);
    //    vec3 smoothModulation = smoothLighting / max(baseColor, vec3(0.08));
    //    finalColor *= mix(vec3(1.0), smoothModulation, 0.22);
    //    vec3 haze = vec3(0.88, 0.93, 0.98);
    //    finalColor = mix(finalColor, haze, 0.20);
    //    float brightness = dot(finalColor, vec3(0.299, 0.587, 0.114));
    //    finalColor += finalColor * pow(brightness, 7.0) * 0.6;
    //    vec3 outlineColor = baseColor * 0.22;
    //    outlineColor = mix(outlineColor, haze, 0.20);
    //    fsColor = vec4(mix(finalColor, outlineColor, edge), 1.0);
    //}
    else if (debugMode == 10) {
        float rawDepth = texture(depthTexture, vUV).r;
        if (rawDepth >= 0.9999) {
            vec3 skyTop     = vec3(0.36, 0.58, 0.88);
            vec3 skyHorizon = vec3(0.74, 0.88, 0.97);
            fsColor = vec4(mix(skyHorizon, skyTop, vUV.y), 1.0);
            return;
        }

        float sobelMask = texture(sobelMaskTexture, vUV).r;
        vec3 normal11   = decodeNormal(encodedNormal);
        float diffuse11 = computeDiffuse(vUV, encodedNormal);

        // Herbe (alpha normalTexture = 0) : diffuse lisse + voile pastel vert doux
        if (texture(normalTexture, vUV).a < 0.5) {
            vec3 grassColor = baseColor * (0.55 + diffuse11 * 0.45);
            grassColor *= mix(vec3(1.0), vec3(1.04, 1.02, 0.94), diffuse11);
            grassColor  = mix(grassColor, vec3(0.84, 0.96, 0.72), 0.13);
            grassColor  = grassColor / (grassColor + vec3(0.85));
            grassColor *= 1.34;
            fsColor = vec4(grassColor, 1.0);
            return;
        }

        // Cel shading cartoon comme mode 10 (bandes + ombres en cercles)
        vec3 litColor = applyLightingWithCircleShadowsCel(baseColor, encodedNormal);

        // Modulation douce de la lumiere (comme mode 10)
        vec3 smoothLight = applyLighting(baseColor, encodedNormal);
        vec3 smoothMod   = smoothLight / max(baseColor, vec3(0.08));
        litColor *= mix(vec3(1.0), smoothMod, 0.22);

        // Bloom doux
        float bright = dot(litColor, vec3(0.299, 0.587, 0.114));
        litColor += litColor * pow(bright, 5.0) * 0.35;

        // Tonemapping Reinhard : evite la surexposition
        litColor  = litColor / (litColor + vec3(0.85));
        litColor *= 1.4;

        // Brouillard atmospherique
        float depth11     = linearDepth(rawDepth);
        float fogFactor11 = smoothstep(20.0, 65.0, depth11);
        litColor = mix(litColor, vec3(0.62, 0.78, 0.95), fogFactor11 * 0.5);

        // Sobel doux, seulement sur les objets marques (sobelMaskTexture)
        float edgeFinal = sobelEdges(vUV) * sobelMask * (1.0 - fogFactor11) * 0.6;
        vec3 outlineColor = litColor * 0.25;
        fsColor = vec4(mix(litColor, outlineColor, edgeFinal), 1.0);
    }
    else {
        fsColor = vec4(1.0, 0.0, 1.0, 1.0);
    }
}
