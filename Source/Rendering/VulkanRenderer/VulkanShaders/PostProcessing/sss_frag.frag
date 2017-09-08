#version 450

layout (set = 0, binding = 0) uniform sampler2D colorTexture;
layout (set = 0, binding = 1) uniform sampler2D depthTexture;
layout (set = 0, binding = 2) uniform sampler2D normalTexture;

layout (location = 0) out vec4 finalColor;

layout (location = 3) in vertexData{
    vec2 uv;
}vertex;

layout (push_constant) uniform pushConstants
{
    vec2 direction;
    float fov;
    float kernelWidth;
    float nearPlane;
    float farPlane;
    float numSamples;
    float nearValues[10];
    float farValues[10];
}constants;

const float e = 2.7183;
const float pi = 3.1415;

const vec3 near = vec3(0.25, 0.08, 0.02);
const vec3 far = vec3(1.0, 0.3, 0.05);
const float w = 0.6;

vec3 maxValue = vec3(0.0);

float getLinearDepth(float depth);
vec3 separableKernel(float x, float w, vec3 near, vec3 far, float depth);
vec3 calculateGaussian(float x, vec3 stdDev);

void main(void)
{
    vec3 inputColor = vec3(0);

    vec4 normal = texture(normalTexture, vertex.uv);
    float sss = normal.a;


    if (sss >= 0.001)
    {
        float depth = getLinearDepth(texture(depthTexture, vertex.uv).r);
        float dx = 0.001 / (tan(constants.fov / 2.0) * depth);
        dx = dx * normal.a / (constants.kernelWidth);

        maxValue = separableKernel(0, w, near, far, depth) * sss;

        for (int i = 1; i < constants.numSamples; i++)
        {
            vec2 offset = vec2(i * dx) * constants.direction;
            float partOfMeshPos = texture(normalTexture, vertex.uv + offset).a;
            float partOfMeshNeg = texture(normalTexture, vertex.uv - offset).a;

            depth = getLinearDepth(texture(depthTexture, vertex.uv + offset).r);
            maxValue += separableKernel(i/(constants.numSamples), w, near, far, depth) * partOfMeshPos;

            depth = getLinearDepth(texture(depthTexture, vertex.uv - offset).r);
            maxValue += separableKernel(-i/(constants.numSamples), w, near, far, depth) * partOfMeshNeg;
        }

        inputColor += texture(colorTexture, vertex.uv).rgb * separableKernel(0, w, near, far, depth) * sss;

        for (int i = 1; i < constants.numSamples; i++)
        {
            vec2 offset = vec2(i * dx) * constants.direction;
            float partOfMeshPos = texture(normalTexture, vertex.uv + offset).a;
            float partOfMeshNeg = texture(normalTexture, vertex.uv - offset).a;

            vec3 color = texture(colorTexture, vertex.uv + offset).rgb;
            depth = getLinearDepth(texture(depthTexture, vertex.uv + offset).r);
            inputColor += color * separableKernel(i/(constants.numSamples), w, near, far, depth) * partOfMeshPos;

            color = texture(colorTexture, vertex.uv - offset).rgb;
            depth = getLinearDepth(texture(depthTexture, vertex.uv - offset).r);
            inputColor += color * separableKernel(-i/(constants.numSamples), w, near, far, depth) * partOfMeshNeg;
        }
        inputColor = inputColor / maxValue;
    }
    else
    {
        inputColor = texture(colorTexture, vertex.uv).rgb;
    }

    finalColor = vec4(inputColor, 1);
}

float getLinearDepth(float depth)
{
    return (2 * constants.nearPlane * constants.farPlane) /
        (constants.nearPlane + constants.farPlane - (depth * (constants.farPlane - constants.nearPlane)));
}

vec3 separableKernel(float x, float w, vec3 near, vec3 far, float depth)
{
    vec3 depthCompensation = vec3(1.0);//pow(vec3(e), (- depth * depth) / (2 * max(near, far)));
    vec3 separableKernelValue = w * calculateGaussian(x, near) + (1 - w) * calculateGaussian(x, far);
    return depthCompensation * separableKernelValue;
}

vec3 calculateGaussian(float x, vec3 stdDev)
{
    return (1 / sqrt(2 * pi * stdDev * stdDev)) * pow(vec3(e), -(x * x) / (2 * stdDev * stdDev));
}