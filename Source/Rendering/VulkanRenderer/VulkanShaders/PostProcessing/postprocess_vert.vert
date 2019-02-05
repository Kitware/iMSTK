#version 460

#extension GL_OVR_multiview : enable

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec2 vertexUV;

layout (location = 3) out vertexData{
    vec2 uv;
    flat uint view;
}vertex;

void main(void)
{
    vertex.uv = vertexUV;
    vertex.view = gl_ViewID_OVR;
    gl_Position = vec4(vertexPosition, 1.0);
}