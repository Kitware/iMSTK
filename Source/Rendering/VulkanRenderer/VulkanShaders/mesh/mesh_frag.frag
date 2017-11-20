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
    vec4 position;
    vec4 color;
    vec4 direction;
    ivec4 state; // 1 type, shadow map index
};

layout (set = 1, binding = 0) uniform globalUniforms
{
    mat4 inverseViewMatrix;
    mat4 inverseProjectionMatrix;
    vec4 resolution;
    light lights[16];
    mat4 lightMatrices[16];
} globals;

layout (set = 1, binding = 1) uniform localUniforms
{
    mat4 transform;
    vec4 color;
    uint receivesShadows;
    float emissivity;
    float roughness;
    float metalness;
} locals;

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
layout (set = 1, binding = 7) uniform sampler2DArray shadowArray;

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
void calculatePBRLighting(vec3 lightDirection, vec3 lightColor, float lightIntensity);
float geometryTerm(vec3 vector);
float squared(float x);
float calculateShadow(int index);

void main(void)
{
    readTextures();

    // If it's 0, then there's a divide by zero error
    roughness = max(roughness * roughness, 0.0001);

    cameraDirection = normalize(vertex.cameraPosition - vertex.position);

    //calculateIndirectLighting();

    vec3 lightRay = vec3(0);
    float lightIntensity = 0;

    for (int i = 0; i < numLights; i++)
    {
        int type = globals.lights[i].state.x;
        if (type > 1)
        {
            lightRay = vertex.position.xyz - globals.lights[i].position.xyz;
            float distanceSquared = dot(lightRay, lightRay);
            lightIntensity = globals.lights[i].color.a / distanceSquared;

            if (type == 3)
            {
                if (globals.lights[i].direction.a > dot(normalize(lightRay), normalize(globals.lights[i].direction.xyz)))
                {
                    lightIntensity = 0;
                }
            }
        }
        else
        {
            lightIntensity = globals.lights[i].color.a * calculateShadow(i);
            lightRay = globals.lights[i].direction.xyz;
        }

        calculatePBRLighting(normalize(lightRay), globals.lights[i].color.rgb, lightIntensity);
    }

    finalDiffuse *= diffuseColor;
    outputColor = vec4(finalDiffuse + (diffuseColor * locals.emissivity), 1);
    outputSpecular = vec4(finalSpecular, 1);

    outputNormal = vec4(normal, subsurfaceScattering);
}

void readTextures()
{
    diffuseColor = locals.color.rgb;
    if (hasDiffuseTexture)
    {
        diffuseColor *= texture(diffuseTexture, vertex.uv).rgb;
    }

    if (hasNormalTexture)
    {
        normal = vertex.TBN * normalize((2.0 * texture(normalTexture, vertex.uv).rgb) - 1.0);
    }
    else
    {
        normal = vertex.TBN * normal;
    }

    if (hasRoughnessTexture)
    {
        roughness *= texture(roughnessTexture, vertex.uv).r;
    }
    else
    {
        roughness = locals.roughness;
    }

    if (hasMetalnessTexture)
    {
        metalness *= texture(metalnessTexture, vertex.uv).r;
    }
    else
    {
        metalness = locals.metalness;
    }

    if (hasSubsurfaceScatteringTexture)
    {
        subsurfaceScattering = texture(subsurfaceScatteringTexture, vertex.uv).r;
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

void calculatePBRLighting(vec3 lightDirection, vec3 lightColor, float lightIntensity)
{
    // Lambert BRDF
    lightDirection *= -1;
    float diffusePow = 1.0 / PI;
    float l_dot_n = max(dot(normal, lightDirection), 0);

    vec3 halfway = normalize(normalize(lightDirection) + cameraDirection);

    // Cook-Torrance BRDF

    // Distribution term: Trowbridge-Reitz
    float roughness_squared = roughness * roughness;
    float D = roughness_squared / (PI * squared(squared(max(dot(halfway, normal), 0)) * (roughness_squared - 1) + 1.001));

    // Fresnel term: Schlick's approximation
    vec3 F_0 = mix(vec3(0.04), diffuseColor, metalness);
    vec3 F = max((F_0) + (1.0 - F_0) * pow(1.0 - max(dot(cameraDirection, halfway), 0), 5), 0);

    // Geometry term: Schlick's GGX
    float G = geometryTerm(cameraDirection) * geometryTerm(lightDirection);

    vec3 specularPow = (D * F * G) /
        (4 * max(dot(normal, cameraDirection), 0.001) * max(dot(normal, lightDirection), 0.001));

    // Energy conservation
    vec3 k_s = F;
    vec3 k_d = (1 - k_s) * (1.0 - metalness);

    finalDiffuse += k_d * diffusePow * lightColor * l_dot_n * lightIntensity;
    finalSpecular += specularPow * specularColor * lightColor * l_dot_n * lightIntensity;
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

float calculateShadow(int index)
{
    int shadowMapIndex = globals.lights[index].state.y;
    if (locals.receivesShadows == 0 || shadowMapIndex == -1)
    {
        return 1;
    }

    float coverage = 0;
    vec4 position = globals.lightMatrices[shadowMapIndex] * vec4(vertex.position, 1);

    if (abs(position.x) >= 1.0 || abs(position.y) >= 1.0 || abs(position.z) >= 1.0)
    {
        return 1;
    }

    position.xy = 0.5 * position.xy + 0.5;

    float resolution = globals.resolution.z;

    for (int x = -2; x < 3; x++)
    {
        for (int y = -2; y < 3; y++)
        {
            float depth = texture(shadowArray, vec3(position.x + x / resolution, position.y + y / resolution, shadowMapIndex)).r;

            if (position.z > depth + 0.002)
            {                
                coverage += 1.0 / 25.0;
            }
        }
    }

    return (1 - coverage);
}