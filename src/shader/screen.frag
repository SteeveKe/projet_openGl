#version 330 core

in vec2 vUV;
out vec4 fsColor;

uniform sampler2D colorTexture;
uniform sampler2D normalTexture;
uniform sampler2D depthTexture;

uniform int debugMode;
uniform vec2 uLightPosition;
uniform float uLightHeight;

const float nearPlane = 0.1;
const float farPlane = 10.0;

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

float halftoneShadow(vec2 fragCoord, float diffuse)
{
    vec2 resolution = vec2(textureSize(colorTexture, 0));

    vec2 pos = fragCoord - 0.5 * resolution;
    pos = rotate2D(radians(20.0)) * pos;

    float gridStep = 8.0;
    vec2 gridPos = mod(pos, gridStep);

    float shadowAmount = 1.0 - diffuse;

    float radius = 0.95 * gridStep * pow(shadowAmount, 2.0);

    float dotMask = circle(
    gridPos,
    vec2(gridStep * 0.5),
    radius
    );

    return dotMask;
}

float computeDiffuse(vec2 uv, vec3 encodedNormal)
{
    vec3 normal = normalize(encodedNormal * 2.0 - 1.0);

    vec2 resolution = vec2(textureSize(colorTexture, 0));

    vec2 lightDelta = uLightPosition - uv;
    lightDelta.x *= resolution.x / resolution.y;

    vec3 lightDirection = normalize(vec3(lightDelta, uLightHeight));

    return max(dot(normal, lightDirection), 0.0);
}

/*
vec3 applyLighting(vec3 baseColor, vec3 encodedNormal)
{
    vec3 normal = normalize(encodedNormal * 2.0 - 1.0);
    vec3 lightDirection = normalize(vec3(0.4, 0.8, 0.6));
    float light = max(dot(normal, lightDirection), 0.0);

    return baseColor * (0.25 + light * 0.75);
}
*/

vec3 applyLighting(vec3 baseColor, vec3 encodedNormal)
{
    float diffuse = computeDiffuse(vUV, encodedNormal);

    float lighting = 0.25 + diffuse * 0.75;

    float dots = halftoneShadow(gl_FragCoord.xy, diffuse);

    float dotDarkness = 0.3;
    float halftoneFactor = mix(1.0, dotDarkness, dots);

    return baseColor * lighting * halftoneFactor;
}

float luminance(vec3 color)
{
    return dot(color, vec3(0.299, 0.587, 0.114));
}

float linearDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
}

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

    vec2 uvTopLeft = uv + texel * vec2(-1.0, 1.0);
    vec2 uvTop = uv + texel * vec2(0.0, 1.0);
    vec2 uvTopRight = uv + texel * vec2(1.0, 1.0);
    vec2 uvLeft = uv + texel * vec2(-1.0, 0.0);
    vec2 uvRight = uv + texel * vec2(1.0, 0.0);
    vec2 uvBottomLeft = uv + texel * vec2(-1.0, -1.0);
    vec2 uvBottom = uv + texel * vec2(0.0, -1.0);
    vec2 uvBottomRight = uv + texel * vec2(1.0, -1.0);

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

    vec3 normalTopLeft = texture(normalTexture, uvTopLeft).rgb * 2.0 - 1.0;
    vec3 normalTop = texture(normalTexture, uvTop).rgb * 2.0 - 1.0;
    vec3 normalTopRight = texture(normalTexture, uvTopRight).rgb * 2.0 - 1.0;
    vec3 normalLeft = texture(normalTexture, uvLeft).rgb * 2.0 - 1.0;
    vec3 normalRight = texture(normalTexture, uvRight).rgb * 2.0 - 1.0;
    vec3 normalBottomLeft = texture(normalTexture, uvBottomLeft).rgb * 2.0 - 1.0;
    vec3 normalBottom = texture(normalTexture, uvBottom).rgb * 2.0 - 1.0;
    vec3 normalBottomRight = texture(normalTexture, uvBottomRight).rgb * 2.0 - 1.0;

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

    float depthTopLeft = linearDepth(texture(depthTexture, uvTopLeft).r) / farPlane;
    float depthTop = linearDepth(texture(depthTexture, uvTop).r) / farPlane;
    float depthTopRight = linearDepth(texture(depthTexture, uvTopRight).r) / farPlane;
    float depthLeft = linearDepth(texture(depthTexture, uvLeft).r) / farPlane;
    float depthRight = linearDepth(texture(depthTexture, uvRight).r) / farPlane;
    float depthBottomLeft = linearDepth(texture(depthTexture, uvBottomLeft).r) / farPlane;
    float depthBottom = linearDepth(texture(depthTexture, uvBottom).r) / farPlane;
    float depthBottomRight = linearDepth(texture(depthTexture, uvBottomRight).r) / farPlane;

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

    //float edgeStrength = colorEdge * 0.7 + normalEdge * 0.4 + depthEdge * 35.0;
    float edgeStrength = colorEdge * 0.7 + normalEdge * 0.2 + depthEdge * 3.0;

    float threshold = 0.18;
    //float antiAliasWidth = 0.02;
    //return smoothstep(threshold - antiAliasWidth, threshold + antiAliasWidth, edgeStrength);
    return step(threshold, edgeStrength);
}



void main()
{
    if (debugMode == 0) {

        // ciel
        float rawDepth = texture(depthTexture, vUV).r;
        if (rawDepth >= 0.9999) { // tres loin
            vec3 skyTop = vec3(0.30, 0.52, 0.80);
            vec3 skyHorizon = vec3(0.72, 0.86, 0.96);
            fsColor = vec4(mix(skyHorizon, skyTop, vUV.y), 1.0);
            return;
        }

        vec3 baseColor = texture(colorTexture, vUV).rgb;
        vec3 encodedNormal = texture(normalTexture, vUV).rgb;
        fsColor = vec4(applyLighting(baseColor, encodedNormal), 1.0);
        //fsColor = vec4(baseColor, 1.0);
    }
    else if (debugMode == 1) {
        fsColor = texture(normalTexture, vUV);
    }
    else if (debugMode == 2) {
        float depth = texture(depthTexture, vUV).r;
        fsColor = vec4(vec3(depth), 1.0);
        //float depth = texture(depthTexture, vUV).r;
        //float visibleDepth = (1.0 - depth) * 20.0;
        //fsColor = vec4(vec3(visibleDepth), 1.0);
    }
    else if (debugMode == 3) {
        float edge = sobelEdges(vUV);
        fsColor = vec4(vec3(edge), 1.0);
    }
    else if (debugMode == 4) {
        float edge = sobelEdges(vUV);

        vec3 encodedNormal = texture(normalTexture, vUV).rgb;

        float diffuse = computeDiffuse(vUV, encodedNormal);

        float dots = halftoneShadow(gl_FragCoord.xy, diffuse);

        float rawDepth = texture(depthTexture, vUV).r;
        float objectMask = 1.0 - step(0.9999, rawDepth);

        dots *= objectMask;

        float result = max(edge, dots);

        fsColor = vec4(vec3(result), 1.0);
    }
    else if (debugMode == 5) {
        vec3 baseColor = texture(colorTexture, vUV).rgb;
        vec3 encodedNormal = texture(normalTexture, vUV).rgb;
        vec3 litColor = applyLighting(baseColor, encodedNormal);
        float edge = sobelEdges(vUV);
        fsColor = vec4(mix(litColor, vec3(0.0), edge), 1.0);
    }
    else if (debugMode == 6) {
        float rawDepth = texture(depthTexture, vUV).r;
        if (rawDepth >= 0.9999) {
            vec3 skyTop     = vec3(0.30, 0.52, 0.80);
            vec3 skyHorizon = vec3(0.90, 0.95, 0.98);
            //vec3 skyHorizon = vec3(0.72, 0.86, 0.96);
            fsColor = vec4(mix(skyHorizon, skyTop, vUV.y), 1.0);
            return;
        }

        vec3 baseColor = texture(colorTexture, vUV).rgb;
        vec3 encodedNormal = texture(normalTexture, vUV).rgb;
        baseColor = applyLighting(baseColor, encodedNormal);

        // 1. couleur ambiante
        baseColor = baseColor * 0.55 + 0.18;

        // 2. léger voile blanc-bleu sur toute la scène
        vec3 haze = vec3(0.88, 0.93, 0.98);
        baseColor = mix(baseColor, haze, 0.1);

        // 3. faux bloom, les zones claires rayonnent un peu
        float brightness = dot(baseColor, vec3(0.299, 0.587, 0.114));
        baseColor += baseColor * pow(brightness, 7.0) * 0.6;

        //// 4. tone mapping doux, évite la surexposition
        //baseColor = baseColor / (baseColor + vec3(0.95));
        //baseColor *= 1.4;

        fsColor = vec4(baseColor, 1.0);
    }
    else {
        fsColor = vec4(1.0, 0.0, 1.0, 1.0);
    }
}
