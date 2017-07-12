// Matrices
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform mat3 normalMatrix;

in vec3 inputPosition;
in vec3 inputNormal;
in vec3 inputTangent;
in vec2 inputUV;

out Vertex
{
    vec3 position;
    vec3 normal;
    vec2 uv;
} vertex;

void main()
{
    mat3 rotationMatrix = mat3(normalMatrix);

    vertex.normal = normalize(rotationMatrix * normalize(inputNormal));
    vec3 position = vec3(modelMatrix * vec4(inputPosition, 1));

    vertex.uv = inputUV;
    vertex.position = position;
    gl_Position = projectionMatrix * viewMatrix * vec4(position, 1);
}