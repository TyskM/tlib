#version 330 core
#define PI 3.1415926535897932384626433832795

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

out vec4 fragColor;

in vec3 vertLocalPos;
in vec3 vertWorldPos;
in vec3 vertNormal;
in vec2 vertTexCoords;
in vec4 vertFragPosLightSpace;
in vec3 vertClipPos;

uniform vec3  cameraPos;
uniform float viewDistance    = 50;

uniform float ambientStrength    = 0.1;
uniform vec3  ambientColor       = vec3(1, 1, 1);
uniform float ambientColorFactor = 0.0;

uniform bool fogEnabled = false;
uniform vec3 fogColor   = vec3(1, 1, 1);

uniform Material material;

// Inject //! #define shadowMapCascadeCount 3
uniform sampler2D csms            [shadowMapCascadeCount];
uniform float     csmEndClipSpace [shadowMapCascadeCount];
in vec4           csmLightClipPos [shadowMapCascadeCount];

uniform sampler2D    _shadowMap;
uniform bool         shadowsEnabled = true;
uniform int          pcfSteps       = 2;
uniform float        minShadowBias  = 0.1;
uniform float        maxShadowBias  = 2.0;

float getFogFactor(float dist, float nearPlane, float farPlane)
{
    float fogMax = farPlane;
    float fogMin = nearPlane;
    if (dist>=fogMax) return 1.0f;
    if (dist<=fogMin) return 0.0f;
    return 1.0f - (fogMax - dist) / (fogMax - fogMin);
}

vec3 schlickFresnel(float vDotH)
{
    vec3 F0 = vec3(0.04);

    vec3 diffuse   = vec3(texture(material.diffuse,  vertTexCoords));
    float metallic = vec3(texture(material.metallic, vertTexCoords)).r;
    F0 = mix(F0, diffuse, metallic);

    return F0 + (1 - F0) * pow(clamp(1.0 - vDotH, 0.001, 1.0), 5);
}

float ggxDistribution(float nDotH)
{
    float  r          = vec3(texture(material.roughness, vertTexCoords)).r;
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

vec2 poissonDisk[18] = vec2[](
vec2(-0.220147, 0.976896),
vec2(-0.735514, 0.693436),
vec2(-0.200476, 0.310353),
vec2( 0.180822, 0.454146),
vec2( 0.292754, 0.937414),
vec2( 0.564255, 0.207879),
vec2( 0.178031, 0.024583),
vec2( 0.613912,-0.205936),
vec2(-0.385540,-0.070092),
vec2( 0.962838, 0.378319),
vec2(-0.886362, 0.032122),
vec2(-0.466531,-0.741458),
vec2( 0.006773,-0.574796),
vec2(-0.739828,-0.410584),
vec2( 0.590785,-0.697557),
vec2(-0.081436,-0.963262),
vec2( 1.000000,-0.100160),
vec2( 0.622430, 0.680868));


float rand(vec4 seed4)
{
    float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
    return fract(sin(dot_product) * 43758.5453);
}

float calcShadow(sampler2D shadowMap, vec4 lightSpace, vec3 lightDir, vec3 normal)
{
    vec3 projCoords = lightSpace.xyz / lightSpace.w; // perform perspective divide
    projCoords = projCoords * 0.5 + 0.5; // transform to [0,1] range
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    float currentDepth = projCoords.z; // get depth of current fragment from light's perspective

    vec2  texelSize        = 1.0 / textureSize(shadowMap, 0);
    float biasDirInfluence = max(maxShadowBias * (1.0 - dot(normal, lightDir)), minShadowBias);
    float biasPx           = texelSize.x * biasDirInfluence; // Texels should be the same size both ways

    float shadow = 0.0;

    // PCF
    for(int x = -pcfSteps; x <= pcfSteps; ++x)
    {
        for(int y = -pcfSteps; y <= pcfSteps; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - biasPx > pcfDepth ? 1.0 : 0.0;
        }    
    }
    int s = pcfSteps*2+1;
    shadow /= s*s;

    // Poisson
    float visibility   = 1.0;
    float req = currentDepth - biasPx;
    int poissonSteps = 4;
    for (int i = 0; i < poissonSteps; i++)
    {
        int   index     = int( 16.0 * rand(vec4(gl_FragCoord.xyy, i))) % 16;
        vec2  offset    = (poissonDisk[index] * 2) * texelSize;
        vec2  samplePos = vec2(projCoords.xy) + offset;
        float sample    = texture(shadowMap, samplePos).r;
        if (sample > req)
            visibility -= 1.0 / poissonSteps;
    }

    shadow *= visibility;

    return shadow;

}

float calcCSMShadowFactor(int cascadeIndex, vec4 lightClipPos, vec3 lightDir, vec3 normal)
{
    return calcShadow(csms[cascadeIndex], lightClipPos, lightDir, normal);
}

vec3 calcPBRLighting(Light light, vec3 posDir, bool isDirLight, vec3 normal)
{
    vec3 lightIntensity = light.color * light.diffuseIntensity;

    vec3 l = vec3(0.0);

    if (isDirLight)
    { l = -posDir.xyz; }
    else
    {
        l = posDir - vertWorldPos;
        float lightToPixelDist = length(l);
        l = normalize(l);
        lightIntensity /= (lightToPixelDist * lightToPixelDist);
    }

    // TODO: Switch to this impl https://learnopengl.com/PBR/Lighting
    vec3 n = normal;                           // Without this vec3 theres ugly black rim
    vec3 v = normalize(cameraPos - vertWorldPos) + (vec3(0,5,0)); // lighting from some angles
    vec3 h = normalize(v + l);                 //\\ TODO: find a less scuffed fix for above.

    float nDotH = max(dot(n, h), 0.0);
    float vDotH = max(dot(v, h), 0.0);
    float nDotL = max(dot(n, l), 0.0);
    float nDotV = max(dot(n, v), 0.0);

    vec3 F = schlickFresnel(vDotH);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;

    vec3 specBRDF_nom  = ggxDistribution(nDotH) * F *
                         geomSmith(nDotL) * geomSmith(nDotV);

    float specBRDF_denom = 4.0 * nDotV * nDotL + 0.0001;

    vec3 specBRDF = specBRDF_nom / specBRDF_denom;

    vec3 fLambert = vec3(0.0);

    float metallic = vec3(texture(material.metallic, vertTexCoords)).r;
    bool isMetal = metallic == 1.0;
    if (!isMetal)
    { fLambert = vec3(texture(material.diffuse, vertTexCoords)); }

    vec3 diffuseBRDF = kD * fLambert / PI;

    vec3 finalColor = (diffuseBRDF + specBRDF) * lightIntensity * nDotL;

    return finalColor;
}

const vec3 cascadeDebugColors[9] = vec3[](
    vec3(1,0,0),
    vec3(1,1,0),
    vec3(1,0,1),
    vec3(0,1,0),
    vec3(1,1,0),
    vec3(0,1,1),
    vec3(0,0,1),
    vec3(0,1,1),
    vec3(1,0,1)
);

vec3 calcPBRDirectionalLight(DirectionalLight light, vec3 normal)
{
    vec3 ret = calcPBRLighting(light.light, light.dir, true, normal);

    if (shadowsEnabled)
    {
        float shadow = 0.0;

        for (int i = 0 ; i < shadowMapCascadeCount; i++)
        {
            if (vertClipPos.z <= csmEndClipSpace[i])
            {
                shadow = calcCSMShadowFactor(i, csmLightClipPos[i], light.dir, normal);
                // Debug
                //ret = mix(ret, cascadeDebugColors[i], 0.03);
                break;
            }
        }

        //float shadow = calcShadow(vertFragPosLightSpace, light.dir, normal);
        ret *= (1.0 - shadow);
    }

    return ret;
}

vec3 calcPBRPointLight(PointLight light, vec3 normal)
{
    return calcPBRLighting(light.light, light.localPos, false, normal);
}

vec3 calcPBRSpotLight(SpotLight light, vec3 normal)
{
    vec3  lightDir    = normalize(light.pos - vertWorldPos);
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
    vec3 color = calcTotalPBRLighting();

    vec3 diffuseSample = vec3(texture(material.diffuse, vertTexCoords));
    vec3 ambient = vec3(ambientStrength) * mix(diffuseSample, ambientColor, ambientColorFactor);
    color += vec3(ambient);

    if (fogEnabled)
    {
        float dist = distance(cameraPos, vertWorldPos);
        float fog = getFogFactor(dist, viewDistance*0.5, viewDistance);
        color = mix(color, fogColor, fog);
    }

    fragColor = vec4(color, 1.0);

}