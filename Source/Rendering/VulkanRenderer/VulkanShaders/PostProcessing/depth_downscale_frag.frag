#version 460

layout (set = 0, binding = 0) uniform sampler2DArray depthTexture;

layout (location = 0) out float finalColor;

layout (location = 3) in vertexData{
    vec2 uv;
    flat uint view;
}vertex;

void main(void)
{
    ivec2 resolution = textureSize(depthTexture, 0).xy;
    ivec3 offset = ivec3(vertex.uv * resolution, vertex.view);

    float minDepth;

    float depth0 = texelFetch(depthTexture, offset + ivec3(0,0,0), 0).r;
    float depth1 = texelFetch(depthTexture, offset + ivec3(0,1,0), 0).r;
    float depth2 = texelFetch(depthTexture, offset + ivec3(1,0,0), 0).r;
    float depth3 = texelFetch(depthTexture, offset + ivec3(1,1,0), 0).r;

    minDepth = min(depth0, depth1);
    minDepth = min(minDepth, depth2);
    minDepth = min(minDepth, depth3);

    finalColor = minDepth;
}