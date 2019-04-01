#version 460

layout (set = 0, binding = 0) uniform sampler2DArray colorTexture;

layout (location = 0) out vec4 finalColor;

layout (location = 3) in vertexData{
    vec2 uv;
    flat uint view;
}vertex;

layout (push_constant) uniform pushConstants
{
    float width;
    float height;
    float numSamples;
    float values[10];
    float offsets[10];
}constants;

void main(void)
{
    vec3 inputColor = vec3(0);
    inputColor += texture(colorTexture, vec3(vertex.uv, vertex.view)).rgb * constants.values[0];

    for (int i = 1; i < constants.numSamples; i++)
    {
        vec3 color = texture(colorTexture, vec3(vertex.uv + vec2(constants.offsets[i] / constants.width, 0), vertex.view)).rgb;
        inputColor += color * constants.values[i];
        color = texture(colorTexture, vec3(vertex.uv - vec2(constants.offsets[i] / constants.width, 0),vertex.view)).rgb;
        inputColor += color * constants.values[i];
    }

    finalColor = vec4(inputColor, 1);
}