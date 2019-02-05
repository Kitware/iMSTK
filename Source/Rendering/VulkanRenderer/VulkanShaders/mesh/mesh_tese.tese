#version 460

// PN triangles implementation

layout (constant_id = 0) const uint numLights = 0;
layout (constant_id = 3) const bool hasNormalTexture = false;

layout (triangles, equal_spacing, cw) in;

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
    flat uint view;
}vertex;

layout (location = 0) in vertexDataTessellation{
    vec3 position;
    vec3 normal;
    vec2 uv;
    mat3 TBN;
    vec3 cameraPosition;
    flat uint view;
}vertexTessellation[];

layout (location = 8) patch in TrianglePatch
{
    vec3 b300;
    vec3 b030;
    vec3 b003;
    vec3 b012;
    vec3 b021;
    vec3 b102;
    vec3 b120;
    vec3 b201;
    vec3 b210;
    vec3 b111;
    vec3 n200;
    vec3 n020;
    vec3 n002;
    vec3 n110;
    vec3 n011;
    vec3 n101;
} trianglePatch;

void main(void)
{
    // Calculate normal
    vec3 normal = vec3(1);
    normal.xyz = trianglePatch.n200 * pow(gl_TessCoord.z, 2)
        + trianglePatch.n020 * pow(gl_TessCoord.x, 2)
        + trianglePatch.n002 * pow(gl_TessCoord.y, 2)
        + trianglePatch.n011 * gl_TessCoord.x * gl_TessCoord.y
        + trianglePatch.n101 * gl_TessCoord.z * gl_TessCoord.y
        + trianglePatch.n110 * gl_TessCoord.z * gl_TessCoord.x;

    // Calculate position
    vec4 position = vec4(1);
    position.xyz = trianglePatch.b300 * pow(gl_TessCoord.z, 3)
        + trianglePatch.b030 * pow(gl_TessCoord.x, 3)
        + trianglePatch.b003 * pow(gl_TessCoord.y, 3)
        + 3 * trianglePatch.b012 * pow(gl_TessCoord.y, 2) * gl_TessCoord.x
        + 3 * trianglePatch.b021 * pow(gl_TessCoord.x, 2) * gl_TessCoord.y
        + 3 * trianglePatch.b102 * pow(gl_TessCoord.y, 2) * gl_TessCoord.z
        + 3 * trianglePatch.b120 * pow(gl_TessCoord.x, 2) * gl_TessCoord.z
        + 3 * trianglePatch.b201 * pow(gl_TessCoord.z, 2) * gl_TessCoord.y
        + 3 * trianglePatch.b210 * pow(gl_TessCoord.z, 2) * gl_TessCoord.x
        + 6 * trianglePatch.b111 * gl_TessCoord.x * gl_TessCoord.y * gl_TessCoord.z;

    // Interpolate vertex
    vertex.normal = normalize(normal);
    vertex.uv = vertexTessellation[0].uv * gl_TessCoord.z
        + vertexTessellation[1].uv * gl_TessCoord.x
        + vertexTessellation[2].uv * gl_TessCoord.y;
    vertex.TBN[0] = normalize(vertexTessellation[0].TBN[0] * gl_TessCoord.z
        + vertexTessellation[1].TBN[0] * gl_TessCoord.x
        + vertexTessellation[2].TBN[0] * gl_TessCoord.y);
    vertex.TBN[1] = normalize(vertexTessellation[0].TBN[1] * gl_TessCoord.z
        + vertexTessellation[1].TBN[1] * gl_TessCoord.x
        + vertexTessellation[2].TBN[1] * gl_TessCoord.y);
    //vertex.TBN[1] = normalize(cross(vertex.normal, vertex.TBN[0]));
    //vertex.TBN[0] = normalize(cross(vertex.TBN[1], vertex.normal));
    vertex.TBN[2] = vertex.normal;
    vertex.cameraPosition = vertexTessellation[0].cameraPosition * gl_TessCoord.z
        + vertexTessellation[1].cameraPosition * gl_TessCoord.x
        + vertexTessellation[2].cameraPosition * gl_TessCoord.y;
    vertex.position = position.xyz;
    vertex.view = vertexTessellation[0].view;

    gl_Position = globals.projectionMatrices[vertex.view] * globals.viewMatrices[vertex.view] * position;
}