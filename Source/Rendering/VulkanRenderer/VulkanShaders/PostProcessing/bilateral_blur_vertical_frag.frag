#version 450

layout (set = 0, binding = 0) uniform sampler2D colorTexture;
layout (set = 0, binding = 1) uniform sampler2D depthTexture;

layout (location = 0) out float finalColor;

layout (location = 3) in vertexData{
    vec2 uv;
}vertex;

layout (push_constant) uniform pushConstants
{
    float width;
    float height;
    float nearPlane;
    float farPlane;
    float numSamples;
    float values[5];
    float offsets[5];
}constants;

const float radius = 0.1;

float getLinearDepth(float depth);

void main(void)
{
    float inputColor = 0;
    inputColor += texture(colorTexture, vertex.uv).r * constants.values[0];

    float centerDepth = getLinearDepth(texture(depthTexture, vertex.uv).r);

    float total = constants.values[0];

    for (int i = 1; i < constants.numSamples; i++)
    {
        float depth = getLinearDepth(texture(depthTexture, vertex.uv + vec2(0, constants.offsets[i] / constants.height)).r);
        float color = texture(colorTexture, vertex.uv + vec2(0, constants.offsets[i] / constants.height)).r;
        float factor = constants.values[i] * max(radius - abs(centerDepth - depth), 0) / radius;
        total += factor;
        inputColor += color * factor;

        depth = getLinearDepth(texture(depthTexture, vertex.uv - vec2(0, constants.offsets[i] / constants.height)).r);
        color = texture(colorTexture, vertex.uv - vec2(0, constants.offsets[i] / constants.height)).r;
        factor = constants.values[i] * max(radius - abs(centerDepth - depth), 0) / radius;
        total += factor;
        inputColor += color * factor;
    }

    finalColor = inputColor / total;
}

float getLinearDepth(float depth)
{
    return (2 * constants.nearPlane * constants.farPlane) /
        (constants.nearPlane + constants.farPlane - (depth * (constants.farPlane - constants.nearPlane)));
}