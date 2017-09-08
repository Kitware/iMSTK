#version 450

layout (location = 0) out vec4 outputColor;
layout (location = 1) out vec4 outputNormal;
layout (location = 2) out vec4 outputSpecular;

layout (constant_id = 0) const uint numLights = 0;
layout (constant_id = 1) const bool tessellation = false;
layout (constant_id = 2) const bool hasDiffuseTexture = false;
layout (constant_id = 3) const bool hasNormalTexture = false;
layout (constant_id = 4) const bool hasSpecularTexture = false;
layout (constant_id = 5) const bool hasRoughnessTexture = false;
layout (constant_id = 6) const bool hasMetalnessTexture = false;
layout (constant_id = 7) const bool hasSubsurfaceScatteringTexture = false;

struct light
{
    vec3 lightVector;
    float lightAngle;
    vec3 lightColor;
    float lightIntensity;
};

layout (set = 1, binding = 0) uniform globalUniforms
{
    light lights[16];
} globals;

layout (location = 0) in vertexData{
    vec3 position;
    vec3 normal;
    vec2 uv;
    mat3 TBN;
    vec3 cameraPosition;
}vertex;

layout (set = 1, binding = 2) uniform sampler2D diffuseTexture;
layout (set = 1, binding = 3) uniform sampler2D normalTexture;
layout (set = 1, binding = 4) uniform sampler2D roughnessTexture;
layout (set = 1, binding = 5) uniform sampler2D metalnessTexture;
layout (set = 1, binding = 6) uniform sampler2D subsurfaceScatteringTexture;
//layout (set = 1, binding = 6) uniform samplerCube irradianceCubemapTexture;

// Constants
const float PI = 3.1415;

// Global variables
vec3 finalDiffuse = vec3(0);
vec3 finalSpecular = vec3(0);
vec3 diffuseColor = vec3(1, 1, 1);
vec3 normal = vec3(0, 0, 1);
float specularValue = 20;
vec3 cameraDirection = vec3(0, 0, 1);
float specularPow = 0;
vec3 specularColor = vec3(1, 1, 1);
float roughness = 1.0;
float metalness = 0.0;
float subsurfaceScattering = 0.0;
vec3 diffuseIndirect = vec3(0, 0, 0);

// Functions
void readTextures();
void calculateIndirectLighting();
void calculateClassicalLighting(vec3 lightDirection, vec3 lightColor);
void calculatePBRLighting(vec3 lightDirection, vec3 lightColor);
float geometryTerm(vec3 vector);
float squared(float x);

void main(void)
{
    readTextures();

    // If it's 0, then there's a divide by zero error
    roughness = max(roughness * roughness, 0.0001);

    cameraDirection = normalize(vertex.cameraPosition - vertex.position);

    //calculateIndirectLighting();

    for (int i = 0; i < numLights; i++)
    {
        calculatePBRLighting(normalize(globals.lights[i].lightVector), globals.lights[i].lightColor * 5);
    }

    finalDiffuse *= diffuseColor;
    outputColor = vec4(finalDiffuse, 1);
    outputSpecular = vec4(finalSpecular, 1);

    outputNormal = vec4(normal, subsurfaceScattering);
}

void readTextures()
{
    float mipLevel = textureQueryLod(diffuseTexture, vertex.uv).x;

    if (hasDiffuseTexture)
    {
        diffuseColor = texture(diffuseTexture, vertex.uv, mipLevel).rgb;
    }

    if (hasNormalTexture)
    {
        normal = vertex.TBN * normalize((2.0 * texture(normalTexture, vertex.uv, mipLevel).rgb) - 1.0);
    }
    else
    {
        normal = vertex.TBN * normal;
    }

    if (hasRoughnessTexture)
    {
        roughness = texture(roughnessTexture, vertex.uv, mipLevel).r;
    }

    if (hasMetalnessTexture)
    {
        metalness = texture(metalnessTexture, vertex.uv, mipLevel).r;
    }

    if (hasSubsurfaceScatteringTexture)
    {
        subsurfaceScattering = texture(subsurfaceScatteringTexture, vertex.uv, mipLevel).r;
    }
}

void calculateIndirectLighting()
{
    // Fresnel term: Schlick's approximation
    vec3 F_0 = mix(vec3(0.04), diffuseColor, metalness);
    vec3 F = (F_0) + (1.0 - F_0) * pow(1.0 - max(dot(cameraDirection, normal), 0), 5);

    // Energy conservation
    vec3 k_s = F;
    vec3 k_d = (1 - k_s) * (1.0 - metalness);

    finalDiffuse += diffuseIndirect * k_d;
}

void calculateClassicalLighting(vec3 lightDirection, vec3 lightColor)
{
    // Lambert BRDF
    float l_dot_n = max(dot(normal, lightDirection), 0);

    vec3 halfway = normalize(lightDirection + cameraDirection);

    // Blinn-Phong BRDF
    float specularity = max(dot(normal, halfway), 0);
    specularPow = pow(specularity, specularValue);
		
    finalDiffuse += lightColor * l_dot_n;
    finalSpecular += specularPow * specularColor * l_dot_n;
}

void calculatePBRLighting(vec3 lightDirection, vec3 lightColor)
{
    // Lambert BRDF
    float diffusePow = 1.0 / PI;
    float l_dot_n = max(dot(normal, lightDirection), 0);

    vec3 halfway = normalize(normalize(lightDirection) + cameraDirection);

    // Cook-Torrance BRDF

    // Distribution term: Trowbridge-Reitz
    float roughness_squared = roughness * roughness;
    float D = roughness_squared / (PI * squared(squared(max(dot(halfway, normal), 0)) * (roughness_squared - 1) + 1));

    // Fresnel term: Schlick's approximation
    vec3 F_0 = mix(vec3(0.04), diffuseColor, metalness);
    vec3 F = (F_0) + (1.0 - F_0) * pow(1.0 - max(dot(cameraDirection, halfway), 0), 5);

    // Geometry term: Schlick's GGX
    float G = geometryTerm(cameraDirection) * geometryTerm(lightDirection);

    vec3 specularPow = (D * F * G) /
        (4 * max(dot(normal, cameraDirection), 0.01) * max(dot(normal, lightDirection), 0.01));

    // Energy conservation
    vec3 k_s = F;
    vec3 k_d = (1 - k_s) * (1.0 - metalness);

    finalDiffuse += k_d * diffusePow * lightColor * l_dot_n;
    finalSpecular += specularPow * specularColor * lightColor * l_dot_n;
}

float geometryTerm(vec3 vector)
{
    float k = roughness / 2;
    float vector_dot_n = max(dot(vector, normal), 0);
    return vector_dot_n / (vector_dot_n * (1 - k) + k);
}

float squared(float x)
{
    return x * x;
}