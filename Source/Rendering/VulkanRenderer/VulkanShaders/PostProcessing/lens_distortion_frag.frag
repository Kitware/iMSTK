#version 460

layout (set = 0, binding = 0) uniform sampler2D colorTexture;

layout (location = 0) out vec4 finalColor;

layout (location = 3) in vertexData{
    vec2 uv;
}vertex;

layout (push_constant) uniform pushConstants
{
    float distortion;
}constants;

// Equation based on:
// Fitzgibbon, Andrew W. "Simulataneous linear estimation of multiple view
// geometry and lens distortion." Proceedings of the 2001 IEEE Computer Society
// Conference on Computer Vision and Pattern Recognition. CVPR 2001. Vol. 1,
// 2001.
vec2 distortCoordinate(vec2 coordinate, float distortion)
{
    // Center coordinate and normalize between -1 and 1
    vec2 c = (coordinate * 2.0) - 1.0;

    // Compute undistorted distance
    float d1 = distance(c, vec2(0));

    // Compute distorted distance
    float d2 = d1 / (1 - (distortion * d1 * d1));

    // Apply distortion
    c *= (d2 / d1);

    // Restore original coordinate space
    c = (c + 1.0) / 2.0;

    return c;
}

void main(void)
{
    vec2 totalLength = distortCoordinate(vec2(0.5, 1.0), constants.distortion);
    vec2 distortedCoordinate = distortCoordinate(vertex.uv, constants.distortion) - vec2(0.5);
    distortedCoordinate /= (distance(totalLength, vec2(0.5)) * 2.0);
    distortedCoordinate += vec2(0.5);

    finalColor = texture(colorTexture, distortedCoordinate);
}