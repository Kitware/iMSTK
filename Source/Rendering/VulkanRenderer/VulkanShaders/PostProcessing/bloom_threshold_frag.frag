#version 450

layout (set = 0, binding = 0) uniform sampler2D colorTexture;

layout (location = 0) out vec4 finalColor;

layout (location = 3) in vertexData{
    vec2 uv;
}vertex;

void main(void)
{
    vec3 inputColor = texture(colorTexture, vertex.uv).rgb;
    finalColor = vec4(inputColor.rgb * vec3(0.05), 1);
}