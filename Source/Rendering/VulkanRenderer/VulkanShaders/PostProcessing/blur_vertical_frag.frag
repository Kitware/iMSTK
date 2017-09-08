#version 450

layout (set = 0, binding = 0) uniform sampler2D colorTexture;

layout (location = 0) out vec4 finalColor;

layout (location = 3) in vertexData{
    vec2 uv;
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
    inputColor += texture(colorTexture, vertex.uv).rgb * constants.values[0];

    for (int i = 1; i < constants.numSamples; i++)
    {
        vec3 color = texture(colorTexture, vertex.uv + vec2(0, constants.offsets[i] / constants.height) ).rgb;
        inputColor += color * constants.values[i];
        color = texture(colorTexture, vertex.uv - vec2(0, constants.offsets[i] / constants.height) ).rgb;
        inputColor += color * constants.values[i];
    }

    finalColor = vec4(inputColor, 1);
}