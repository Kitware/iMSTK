#version 450

layout (set = 0, binding = 0) uniform sampler2D colorTexture;

layout (location = 0) out vec4 finalColor;

layout (location = 3) in vertexData{
    vec2 uv;
}vertex;

void main(void)
{
    vec4 inputColor = texture(colorTexture, vertex.uv);
    finalColor = inputColor * vec4(1,0,0,1);
}