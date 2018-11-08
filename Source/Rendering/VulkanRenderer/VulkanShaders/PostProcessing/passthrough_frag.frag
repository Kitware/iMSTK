#version 450

layout (set = 0, binding = 0) uniform sampler2D inputTexture;

layout (location = 0) out vec4 finalColor;

layout (location = 3) in vertexData{
    vec2 uv;
}vertex;

void main(void)
{
    ivec2 resolution = textureSize(inputTexture, 0);
    ivec2 offset = ivec2(vertex.uv * resolution);

    finalColor = texture(inputTexture, vertex.uv);
}