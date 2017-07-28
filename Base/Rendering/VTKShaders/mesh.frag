out vec4 gl_FragData[0];

uniform vec3 cameraPosition = vec3(0, 0, 1);
uniform int numLights = 0;

in Vertex
{
    vec3 position;
    vec3 normal;
    vec2 uv;
} vertex;

// Lights
uniform vec3 lightPosition[16];
uniform int lightType[16];
uniform vec4 lightColor[16];
uniform vec4 lightDirection[16];

// Textures
#ifdef SHADED
uniform sampler2D diffuseTexture;
uniform samplerCube cubemapTexture;
uniform vec3 diffuseColorUniform = vec3(1);
#else
uniform vec3 debugColor = vec3(0);
#endif

// Material properties
uniform float metalness = 0;
uniform float roughness = 1.0;

// Colors
vec3 diffuseColor = vec3(1);
vec3 reflectionColor = vec3(1);
vec3 specularColor = vec3(0);
vec3 finalSpecular = vec3(0);
vec3 finalDiffuse = vec3(0);
vec3 finalColor = vec3(0);

// Other information
vec3 normal = vec3(0, 0, 1);
vec3 cameraDirection = vec3(0, 0, 1);

void calculateClassicalLighting(vec3 lightDirection, vec3 color, float intensity);
vec3 toneMap(vec3 color);

void main()
{
#ifdef SHADED
    normal = normalize(vertex.normal);
    cameraDirection = normalize(vertex.position - cameraPosition);

#ifdef DIFFUSE_TEXTURE
    diffuseColor = pow(texture(diffuseTexture, vertex.uv).rgb, vec3(2.2));
#else
    diffuseColor = diffuseColorUniform;
#endif
#ifdef CUBEMAP_TEXTURE
    reflectionColor = pow(texture(cubemapTexture, reflect(cameraDirection, normal)).rgb, vec3(2.2));
#endif

    specularColor = mix(vec3(1), diffuseColor, metalness);
    diffuseColor = mix(diffuseColor, reflectionColor * diffuseColor, metalness);

    float lightIntensity = 0;

    vec3 lightDir = vec3(0);
    for (int i = 0; i < numLights; i++)
    {
        if (lightType[i] > 1)
        {
            lightDir = vertex.position.xyz - lightPosition[i];
            float distance_squared = dot(lightDir, lightDir);
            lightIntensity = lightColor[i].a / distance_squared;

            if (lightType[i] == 3)
            {
                if (lightDirection[i].a > dot(normalize(lightDir), normalize(lightDirection[i].xyz)))
                {
                    lightIntensity = 0;
                }
            }
        }
        else
        {
            lightDir = lightDirection[i].xyz;
            lightIntensity = lightColor[i].a;
        }
        calculateClassicalLighting(normalize(lightDir),
            lightColor[i].rgb, lightIntensity);
    }

    // Sum components and apply gamma correction
    finalColor = mix(0.5, 1.0, roughness) * finalDiffuse + mix(0.5, 0.0, roughness) * finalSpecular;

    // Apply tone mapping
    finalColor = toneMap(finalColor);

    gl_FragData[0] = vec4(pow(finalColor, vec3(1.0 / 2.2)), 1);
#else
    gl_FragData[0] = vec4(debugColor, 1);
#endif
}

void calculateClassicalLighting(vec3 lightDirection, vec3 lightColor, float lightIntensity)
{
    float diffusePow = max(dot(normal, -lightDirection), 0.0);
    finalDiffuse += max(lightColor * lightIntensity * diffuseColor * diffusePow, 0.0);

    vec3 halfway = normalize(-cameraDirection - lightDirection);
    float specularPow = max(dot(normal, halfway), 0.0);
    finalSpecular += specularColor * lightColor * lightIntensity * vec3(pow(specularPow, (1.0 - roughness) * 100.0));
}

// Reinhard tonemapping
vec3 toneMap(vec3 color)
{
    float luminance = dot(color, vec3(.21, .72, .07));
    return (luminance / (luminance + vec3(1.0))) * color;
}