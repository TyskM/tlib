#version 330 core
#define PI 3.1415926535897932384626433832795

out vec4 fragColor;

in vec3 vertLocalPos;
in vec3 vertWorldPos;
in vec3 vertNormal;
in vec2 vertTexCoords;

in vec3 vertFragPos;
in vec3 vertLightPos;

uniform vec3  fragCameraPos;
uniform float ambientStrength  = 0.1;

struct Material
{
    sampler2D diffuse;
    sampler2D roughness;
    sampler2D metallic;
};

struct Attenuation
{
    float constant;
    float linear;
    float expo;
};

struct Light
{
    vec3  color;
    float ambientIntensity;
    float diffuseIntensity;
};

struct PointLight
{
    Light light;
    vec3  localPos; // used for lighting calculations
    vec3  worldPos; // used for point light shadow mapping
};

struct DirectionalLight
{
    Light light;
    vec3  dir;
};

struct SpotLight
{
    Light light;
    vec3  pos;
    vec3  dir;
    float cosAngle;
    float outerCosAngle;
};

// Inject    //! #define maxDirectionalLights
uniform int              directionalLightCount;
uniform DirectionalLight directionalLights[maxDirectionalLights];

// Inject    //! #define maxPointLights
uniform int              pointLightCount;
uniform PointLight       pointLights[maxPointLights];

// Inject    //! #define maxSpotLights
uniform int              spotLightCount;
uniform SpotLight        spotLights[maxSpotLights];

uniform Material material;

vec3 schlickFresnel(float vDotH)
{
    vec3 F0 = vec3(0.04);

    bool isMetal = vec3(texture(material.metallic, vertTexCoords)).r == 1.0;
    if (isMetal)
    { F0 = vec3(texture(material.diffuse, vertTexCoords)); }

    vec3 ret = F0 + (1 - F0) * pow(clamp(1.0 - vDotH, 0.0, 1.0), 5);

    return ret;
}

float ggxDistribution(float nDotH)
{
    float  r = vec3(texture(material.roughness, vertTexCoords)).r;
    float  alpha2     = r * r * r * r;
    float  d          = nDotH * nDotH * (alpha2 - 1) + 1;
    float  ggxdistrib = alpha2 / (PI * d * d);
    return ggxdistrib;
}

float geomSmith(float dp)
{
    float  r = vec3(texture(material.roughness, vertTexCoords)).r;
    float  k = (r + 1.0) * (r + 1.0) / 8.0;
    float  denom = dp * (1 - k) + k;
    return dp / denom;
}

vec3 calcPBRLighting(Light light, vec3 posDir, bool isDirLight, vec3 normal)
{
    vec3 lightIntensity = light.color * light.diffuseIntensity;

    vec3 l = vec3(0.0);

    if (isDirLight) {
        l = -posDir.xyz;
    } else {
        l = posDir - vertLocalPos;
        float lightToPixelDist = length(l);
        l = normalize(l);
        lightIntensity /= (lightToPixelDist * lightToPixelDist);
    }

    vec3 n = normal;
    vec3 v = normalize(fragCameraPos - vertLocalPos);
    vec3 h = normalize(v + l);

    float nDotH = max(dot(n, h), 0.0);
    float vDotH = max(dot(v, h), 0.0);
    float nDotL = max(dot(n, l), 0.0);
    float nDotV = max(dot(n, v), 0.0);

    vec3 F = schlickFresnel(vDotH);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;

    vec3 specBRDF_nom  = ggxDistribution(nDotH) *
                         F *
                         geomSmith(nDotL) *
                         geomSmith(nDotV);

    float specBRDF_denom = 4.0 * nDotV * nDotL + 0.0001;

    vec3 specBRDF = specBRDF_nom / specBRDF_denom;

    vec3 fLambert = vec3(0.0);

    bool isMetal = vec3(texture(material.metallic, vertTexCoords)).r == 1.0;
    if (!isMetal)
    { fLambert = vec3(texture(material.diffuse, vertTexCoords)); }

    vec3 diffuseBRDF = kD * fLambert / PI;

    vec3 finalColor = (diffuseBRDF + specBRDF) * lightIntensity * nDotL;

    return finalColor;
}

vec3 calcPBRDirectionalLight(DirectionalLight light, vec3 normal)
{
    return calcPBRLighting(light.light, light.dir, true, normal);
}

vec3 calcPBRPointLight(PointLight light, vec3 normal)
{
    return calcPBRLighting(light.light, light.localPos, false, normal);
}

vec3 calcPBRSpotLight(SpotLight light, vec3 normal)
{
    vec3  lightDir    = normalize(light.pos - vertFragPos);
    float theta       = dot(lightDir, normalize(-light.dir));
    float epsilon     = light.cosAngle - light.outerCosAngle;
    float intensity   = clamp((theta - light.outerCosAngle) / epsilon, 0.0, 1.0);    

    return calcPBRLighting(light.light, light.pos, false, normal) * intensity;

}

vec3 calcTotalPBRLighting()
{
    vec3 normal = normalize(vertNormal);

    vec3 totalLight = vec3(0, 0, 0);

    for (int i = 0; i < directionalLightCount; i++)
    {
        totalLight += calcPBRDirectionalLight(directionalLights[i], normal);
    }

    for (int i = 0; i < pointLightCount; i++)
    {
        totalLight += calcPBRPointLight(pointLights[i], normal);
    }

    for (int i = 0; i < spotLightCount; i++)
    {
        totalLight += calcPBRSpotLight(spotLights[i], normal);
    }

    // HDR tone mapping
    totalLight = totalLight / (totalLight + vec3(1.0));

    // Gamma correction
    vec3 finalLight = vec3( pow(totalLight, vec3(1.0/2.2)) );

    return finalLight;
}

void main()
{
    //fragColor = vec4(blinnPhongDir(vertNormal, vertFragPos, sunDir, lightColor), 1.0);

    vec3 color = calcTotalPBRLighting();
    vec3 ambient = vec3(ambientStrength) * vec3(texture(material.diffuse, vertTexCoords));
    color += vec3(ambient);
    fragColor = vec4(color, 1.0);
}