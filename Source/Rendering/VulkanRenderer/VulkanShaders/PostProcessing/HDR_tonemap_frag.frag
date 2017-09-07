#version 450

layout (set = 0, binding = 0) uniform sampler2D colorTexture;

layout (location = 0) out vec4 finalColor;

layout (location = 3) in vertexData{
    vec2 uv;
}vertex;

// Reinhard tonemapping
vec3 tonemap(vec3 color)
{
    vec3 tempColor = (color / (color + 1));
    return tempColor;
}

// John Hable's filmic tonemapping
vec3 tonemapTwoHelper(vec3 color)
{
    float shoulderValue = 0.22;
    float linearValue = 0.3;
    float linearDir = 0.1;
    float toeValue = 0.2;
    float toeUpper = 0.001;
    float toeLower = 0.3;

    vec3 x = color * (color * shoulderValue + linearValue * linearDir) + toeValue * toeUpper;
    vec3 y = color * (color * shoulderValue + linearValue) + toeValue * toeLower;
    float z = toeUpper/toeLower;

    return (x / y) - z;
}

vec3 tonemapTwo(vec3 color)
{
    vec3 whiteColor = vec3(1.0);
    vec3 tempColor = tonemapTwoHelper(color);
    vec3 normalizedColor = tonemapTwoHelper(whiteColor);

    return tempColor / whiteColor;
}

void main(void)
{
    vec4 inputColor = texture(colorTexture, vertex.uv);
    finalColor = vec4(tonemapTwo(inputColor.rgb), inputColor.a);
}