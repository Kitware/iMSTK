#version 450

// PN triangles implementation

layout (constant_id = 0) const uint numLights = 0;
layout (constant_id = 3) const bool hasNormalTexture = false;

layout (vertices = 3) out;

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

layout(location = 0) in vertexData{
    vec3 position;
    vec3 normal;
    vec2 uv;
    mat3 TBN;
    vec3 cameraPosition;
}vertex[];

layout (location = 0) out vertexDataTessellation{
    vec3 position;
    vec3 normal;
    vec2 uv;
    mat3 TBN;
    vec3 cameraPosition;
}vertexTessellation[];

layout (location = 7) patch out TrianglePatch
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

float projectPosition(vec3 point, vec3 planePoint, vec3 planeNormal)
{
    return dot(point - planePoint, planeNormal);
}

float projectNormal(vec3 point1, vec3 point2, vec3 normal1, vec3 normal2)
{
    vec3 difference = point2 - point1;
    return dot(difference, normal1 + normal2) / dot(difference, difference);
}

vec3 computeEdgeControlPosition(int index1, int index2)
{
    vec3 point1 = vertex[index1].position;
    vec3 point2 = vertex[index2].position;
    vec3 normal1 = vertex[index1].normal;
    return (2 * point1 + point2 - projectPosition(point2, point1, normal1) * normal1) / 3;
}

vec3 computeEdgeControlNormal(int index1, int index2)
{
    vec3 point1 = vertex[index1].position;
    vec3 point2 = vertex[index2].position;
    vec3 normal1 = vertex[index1].normal;
    vec3 normal2 = vertex[index2].normal;
    return normalize(normal1 + normal2 - projectNormal(point1, point2, normal1, normal2) * (point2 - point1));
}

void main(void)
{
    // Pass through vertex values
    vertexTessellation[gl_InvocationID].position = vertex[gl_InvocationID].position;
    vertexTessellation[gl_InvocationID].normal = vertex[gl_InvocationID].normal;
    vertexTessellation[gl_InvocationID].uv = vertex[gl_InvocationID].uv;
    vertexTessellation[gl_InvocationID].TBN = vertex[gl_InvocationID].TBN;
    vertexTessellation[gl_InvocationID].cameraPosition = vertex[gl_InvocationID].cameraPosition;

    // Calculate patch control point positions
    trianglePatch.b300 = vertex[0].position;
    trianglePatch.b030 = vertex[1].position;
    trianglePatch.b003 = vertex[2].position;
    trianglePatch.b012 = computeEdgeControlPosition(2, 1);
    trianglePatch.b021 = computeEdgeControlPosition(1, 2);
    trianglePatch.b102 = computeEdgeControlPosition(2, 0);
    trianglePatch.b120 = computeEdgeControlPosition(1, 0);
    trianglePatch.b201 = computeEdgeControlPosition(0, 2);
    trianglePatch.b210 = computeEdgeControlPosition(0, 1);
    vec3 averageEdge = (trianglePatch.b012 + trianglePatch.b021
        + trianglePatch.b102 + trianglePatch.b120
        + trianglePatch.b201 + trianglePatch.b210) / 6;
    vec3 averageVertex = (trianglePatch.b300 + trianglePatch.b030 + trianglePatch.b003) / 3;
    trianglePatch.b111 = averageEdge + (averageEdge - averageVertex) / 2;

    // Calculate patch control point normals
    trianglePatch.n200 = vertex[0].normal;
    trianglePatch.n020 = vertex[1].normal;
    trianglePatch.n002 = vertex[2].normal;
    trianglePatch.n110 = computeEdgeControlNormal(0, 1);
    trianglePatch.n011 = computeEdgeControlNormal(1, 2);
    trianglePatch.n101 = computeEdgeControlNormal(2, 1);

    // Determines how much to tessellate
    if (gl_InvocationID == 0)
    {
        gl_TessLevelOuter[0] = 3;
        gl_TessLevelOuter[1] = 3;
        gl_TessLevelOuter[2] = 3;
        gl_TessLevelInner[0] = 3;
    }

    //vertexTessellation[0] = vertex[0];
}