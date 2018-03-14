#version 450

layout (constant_id = 0) const uint numLights = 0;
layout (constant_id = 1) const bool tessellation = false;
layout (constant_id = 2) const bool hasDiffuseTexture = false;
layout (constant_id = 3) const bool hasNormalTexture = false;
layout (constant_id = 4) const bool hasSpecularTexture = false;
layout (constant_id = 5) const bool hasRoughnessTexture = false;
layout (constant_id = 6) const bool hasMetalnessTexture = false;
layout (constant_id = 7) const bool hasSubsurfaceScatteringTexture = false;

struct light
{
    vec4 position;
    vec4 color;
    vec4 direction;
    ivec4 state; // 1 type, shadow map index
};

layout (set = 1, binding = 0) uniform globalUniforms
{
    mat4 inverseViewMatrix;
    mat4 inverseProjectionMatrix;
    vec4 resolution;
    light lights[16];
    mat4 lightMatrices[16];
} globals;

layout (set = 1, binding = 1) uniform localUniforms
{
    vec4 color;
    mat4 transform;
} locals;

void main(void)
{
}