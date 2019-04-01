#version 460

layout (set = 0, binding = 0) uniform sampler2DArray colorTexture;

layout (location = 0) out vec4 finalColor;

layout (location = 3) in vertexData{
    vec2 uv;
    flat uint view;
}vertex;

void main(void)
{
    vec3 inputColor = texture(colorTexture, vec3(vertex.uv, vertex.view)).rgb;
    finalColor = vec4(inputColor.rgb * vec3(0.05), 1);
}