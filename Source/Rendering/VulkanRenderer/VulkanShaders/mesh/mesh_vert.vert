#version 460

#extension GL_OVR_multiview : enable

layout (constant_id = 0) const uint numLights = 0;
layout (constant_id = 1) const bool tessellation = false;
layout (constant_id = 3) const bool hasNormalTexture = false;

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec3 vertexTangent;
layout (location = 3) in vec2 vertexUV;
layout (location = 4) in vec3 vertexColor;

struct light
{
    vec3 lightVector;
    float lightAngle;
    vec3 lightColor;
    float lightIntensity;
};

layout (set = 0, binding = 0) uniform globalUniforms
{
    mat4 projectionMatrices[2];
    mat4 viewMatrices[2];
    vec4 cameraPositions[2];
    light lights[16];
} globals;

layout (set = 0, binding = 1) uniform localUniforms
{
    mat4 transform;
} locals;

layout (location = 0) out vertexData{
    vec3 position;
    vec3 normal;
    vec2 uv;
    mat3 TBN;
    vec3 cameraPosition;
    vec3 color;
    flat uint view;
}vertex;

void main(void)
{
    vec4 position = locals.transform * vec4(vertexPosition, 1.0);
    vec4 normal = normalize(locals.transform * vec4(normalize(vertexNormal), 0.0));
    vec4 tangent = normalize(locals.transform * vec4(normalize(vertexTangent), 0.0));

    vec4 bitangent = vec4(cross(normal.xyz, tangent.xyz), 0.0);
    tangent = vec4(cross(bitangent.xyz, normal.xyz), 0.0);

    vertex.TBN = mat3(tangent.xyz, bitangent.xyz, normal.xyz);

    vertex.cameraPosition = globals.cameraPositions[gl_ViewID_OVR].xyz;
    vertex.position = position.xyz;
    vertex.normal = normalize(normal.xyz);
    vertex.uv = vertexUV;
    vertex.color = vertexColor;
    vertex.view = gl_ViewID_OVR;

    gl_Position = globals.projectionMatrices[gl_ViewID_OVR] * globals.viewMatrices[gl_ViewID_OVR] * position;
}