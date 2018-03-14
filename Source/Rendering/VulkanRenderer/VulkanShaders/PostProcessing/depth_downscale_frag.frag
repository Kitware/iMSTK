#version 450

layout (set = 0, binding = 0) uniform sampler2D depthTexture;

layout (location = 0) out float finalColor;

layout (location = 3) in vertexData{
    vec2 uv;
}vertex;

void main(void)
{
    ivec2 resolution = textureSize(depthTexture, 0);
    ivec2 offset = ivec2(vertex.uv * resolution);

    float minDepth;

    float depth0 = texelFetch(depthTexture, offset + ivec2(0,0), 0).r;
    float depth1 = texelFetch(depthTexture, offset + ivec2(0,1), 0).r;
    float depth2 = texelFetch(depthTexture, offset + ivec2(1,0), 0).r;
    float depth3 = texelFetch(depthTexture, offset + ivec2(1,1), 0).r;

    minDepth = min(depth0, depth1);
    minDepth = min(minDepth, depth2);
    minDepth = min(minDepth, depth3);

    finalColor = minDepth;
}