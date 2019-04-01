#version 460

layout (set = 0, binding = 0) uniform sampler2DArray depthTexture;
layout (set = 0, binding = 1) uniform sampler2D noiseTexture;

layout (location = 0) out vec4 finalColor;

layout (location = 3) in vertexData{
    vec2 uv;
    flat uint view;
}vertex;

layout (push_constant) uniform pushConstants
{
    float fov;
    float kernelWidth;
    float nearPlane;
    float farPlane;
    float numSamples;
    vec2 resolution;
}constants;

const float pi = 3.1415;
const float tangentBias = pi / 16;

float getLinearDepth(float depth);
float computeAO(vec2 direction, float fragmentStepSize, float realStepSize, float numSteps, vec3 normal, float depth);

void main(void)
{
    float ambientValue = 0.0;
    float samples = constants.numSamples;
    float width = constants.kernelWidth;
    float depth = getLinearDepth(texture(depthTexture, vec3(vertex.uv, vertex.view)).r);
    float fragmentSize = 1.0 / (tan(constants.fov / 2.0) * depth);
    float fragmentStepSize = fragmentSize * (constants.kernelWidth / samples);
    float realStepSize = constants.kernelWidth / samples;
    ivec2 sampleLocation = ivec2(mod(gl_FragCoord.xy, 128.0));
    vec4 noise = texelFetch(noiseTexture, sampleLocation, 0);

    vec2 neighborOffset = vec2(1.0) / constants.resolution;
    float x_up = getLinearDepth(texture(depthTexture, vec3(vertex.uv + vec2(neighborOffset.x, 0), vertex.view)).r);
    float x_down = getLinearDepth(texture(depthTexture, vec3(vertex.uv + vec2(-neighborOffset.x, 0), vertex.view)).r);
    float y_up = getLinearDepth(texture(depthTexture, vec3(vertex.uv + vec2(0, neighborOffset.y), vertex.view)).r);
    float y_down = getLinearDepth(texture(depthTexture, vec3(vertex.uv + vec2(0, -neighborOffset.y), vertex.view)).r);

    float dx, dy;

    // Normal extraction from depth buffer
    if (abs(x_up - depth) < abs(x_down - depth))
    {
        dx = x_up - depth;
    }
    else
    {
        dx = depth - x_down;
    }

    if (abs(y_up - depth) < abs(y_down - depth))
    {
        dy = y_up - depth;
    }
    else
    {
        dy = depth - y_down;
    }

    vec3 normal = cross(normalize(vec3(fragmentSize, 0, dx)), normalize(vec3(0, fragmentSize, dy)));

    float rotationAngle = noise.r * 2 * pi;
    vec2 rotation = vec2(cos(rotationAngle), sin(rotationAngle));

    ambientValue += computeAO(vec2(0.87, 0.5) * rotation, fragmentStepSize, realStepSize, samples, normal, depth);
    ambientValue += computeAO(vec2(-0.87, 0.5) * rotation, fragmentStepSize, realStepSize, samples, normal, depth);
    ambientValue += computeAO(vec2(0.87, -0.5) * rotation, fragmentStepSize, realStepSize, samples, normal, depth);
    ambientValue += computeAO(vec2(-0.87, -0.5) * rotation, fragmentStepSize, realStepSize, samples, normal, depth);
    ambientValue += computeAO(vec2(0, -1) * rotation, fragmentStepSize, realStepSize, samples, normal, depth);
    ambientValue += computeAO(vec2(0, 1) * rotation, fragmentStepSize, realStepSize, samples, normal, depth);

    finalColor = vec4(vec3(1 - (ambientValue / 6)), 1);
}

float getLinearDepth(float depth)
{
    return (2 * constants.nearPlane * constants.farPlane) /
        (constants.nearPlane + constants.farPlane - (depth * (constants.farPlane - constants.nearPlane)));
}

float computeAO(vec2 direction, float fragmentStepSize, float realStepSize, float numSteps, vec3 normal, float depth)
{
    float ao = 0;
    float lastAO = 0;
    float fragmentOffset = 0;
    float realOffset = 0;
    vec3 tangent = cross(normal, vec3(direction, 0));
    tangent = cross(tangent, normal);
    float horizonDistance = 0;
    float tangentAngle = atan(tangent.z, length(tangent.xy)) + tangentBias;
    float maxHorizonAngle = tangentAngle;
    float horizonLength = 0;
    float sampleRadius = 0;
    float maxRadius = realStepSize * numSteps;

    for (int i = 1; i <= numSteps; i++)
    {
        fragmentOffset += fragmentStepSize;
        realOffset += realStepSize;

        vec2 textureOffset = direction * fragmentOffset;
        float depthSample = getLinearDepth(texture(depthTexture, vec3(vertex.uv + textureOffset, vertex.view)).r);
        float depthDifference = depthSample - depth;

        float horizonAngle = atan(-depthDifference, -realOffset);

        sampleRadius = length(vec2(realOffset, depthDifference));
        if (maxHorizonAngle < horizonAngle)
        {
            float radiusWeight = sampleRadius / maxRadius;

            maxHorizonAngle = horizonAngle;

            float tempAO = sin(maxHorizonAngle) - sin(tangentAngle);
            ao += (tempAO - lastAO) * (1 - radiusWeight * radiusWeight);
            lastAO = tempAO;
        }
    }

    ao = clamp(ao, 0.0, 1.0);
    return ao;
}