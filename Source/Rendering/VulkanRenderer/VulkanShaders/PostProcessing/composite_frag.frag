#version 460

layout (set = 0, binding = 0) uniform sampler2DArray colorTexture1;
layout (set = 0, binding = 1) uniform sampler2DArray colorTexture2;

layout (location = 0) out vec4 finalColor;

layout (location = 3) in vertexData{
    vec2 uv;
    flat uint view;
}vertex;

void main(void)
{
    vec4 color1 = texture(colorTexture1, vec3(vertex.uv, vertex.view));
    vec4 color2 = texture(colorTexture2, vec3(vertex.uv, vertex.view));
    finalColor = vec4(color1.rgb + color2.rgb, color1.a);
}