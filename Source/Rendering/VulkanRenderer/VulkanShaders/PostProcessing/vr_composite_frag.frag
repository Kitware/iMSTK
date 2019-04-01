#version 460

layout (set = 0, binding = 0) uniform sampler2D colorTexture0;
layout (set = 0, binding = 1) uniform sampler2D colorTexture1;

layout (location = 0) out vec4 finalColor;

layout (location = 3) in vertexData{
    vec2 uv;
    flat uint view;
}vertex;

layout(push_constant) uniform pushConstants
{
    float numViews;
}constants;

void main(void)
{
    vec4 color;
    int numViews = int(round(constants.numViews));

    if (numViews == 2)
    {
        if (vertex.uv.x >= 0.5)
        {
            color = texture(colorTexture1, vec2(vertex.uv.x * 2.0, vertex.uv.y));
        }
        else
        {
            color = texture(colorTexture0, vec2((vertex.uv.x - 0.5) * 2.0, vertex.uv.y));
        }
    }
    else
    {
        color = texture(colorTexture0, vertex.uv);
    }

    finalColor = vec4(color);
}