#version 450

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec2 vertexUV;

layout (location = 3) out vertexData{
    vec2 uv;
}vertex;

void main(void)
{
    vertex.uv = vertexUV;
    gl_Position = vec4(vertexPosition, 1.0);
}