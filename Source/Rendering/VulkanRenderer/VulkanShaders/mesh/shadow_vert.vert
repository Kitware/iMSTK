#version 450

layout (constant_id = 0) const uint numLights = 0;
layout (constant_id = 1) const bool tessellation = false;
layout (constant_id = 3) const bool hasNormalTexture = false;

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec3 vertexTangent;
layout (location = 3) in vec2 vertexUV;

struct light
{
    vec3 lightVector;
    float lightAngle;
    vec3 lightColor;
    float lightIntensity;
};

layout (set = 0, binding = 0) uniform globalUniforms
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
    vec4 cameraPosition;
    light lights[16];
} globals;

layout (set = 0, binding = 1) uniform localUniforms
{
    mat4 transform;
} locals;

layout (push_constant) uniform pushConstants
{
    mat4 projectionViewMatrix;
}constants;

void main(void)
{
    vec4 position = locals.transform * vec4(vertexPosition, 1.0);

    gl_Position = constants.projectionViewMatrix * position;
}