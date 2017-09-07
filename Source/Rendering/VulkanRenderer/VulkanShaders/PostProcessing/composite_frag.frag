#version 450

layout (set = 0, binding = 0) uniform sampler2D colorTexture1;
layout (set = 0, binding = 1) uniform sampler2D colorTexture2;

layout (location = 0) out vec4 finalColor;

layout (location = 3) in vertexData{
    vec2 uv;
}vertex;

void main(void)
{
    vec4 color1 = texture(colorTexture1, vertex.uv);
    vec4 color2 = texture(colorTexture2, vertex.uv);
    finalColor = vec4(color1.rgb + color2.rgb, color1.a);
}