#version 330 core
out vec4 fragColor;

in vec3 vertLocalPos;
in vec3 vertWorldPos;
in vec3 vertNormal;
in vec2 vertTexCoords;

in vec3 vertFragPos;
in vec3 vertLightPos;

uniform vec3  fragCameraPos;
uniform vec3  lightColor       = vec3(0.2, 0.2, 0.2);
uniform float ambientStrength  = 0.1;
uniform vec3  sunDir           = vec3(-0.2, -1.0, -0.3);

#define PI 3.1415926535897932384626433832795

struct Material
{
    sampler2D diffuse;
    sampler2D roughness;
    sampler2D metallic;
};

struct BaseLight
{
    vec3  Color;
    float AmbientIntensity;
    float DiffuseIntensity;
};

uniform Material material;

//vec3 blinnPhongDir(vec3 normal, vec3 fragPos, vec3 lightDirRaw, vec3 lightColor)
//{
//    // ambient
//    vec3 ambient = lightColor * vec3(texture(material.diffuse, vertTexCoords));
//
//    // diffuse
//    vec3  lightDir = normalize(-lightDirRaw);
//    float diff     = max(dot(lightDir, normal), 0.0);
//    vec3  diffuse  = diff * lightColor * vec3(texture(material.diffuse, vertTexCoords));
//
//    // specular
//    vec3 viewDir    = normalize(fragCameraPos - fragPos);
//    float spec      = 0.0;
//    vec3 halfwayDir = normalize(lightDir + viewDir);  
//    spec            = pow(max(dot(normal, halfwayDir), 0.0), 1.0 - material.roughness);
//    vec3 specular   = spec * lightColor * vec3(texture(material.specular, vertTexCoords)); 
//   
//    return ambient + diffuse + specular;
//}

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

vec3 CalcPBRLighting(BaseLight Light, vec3 PosDir, bool IsDirLight, vec3 Normal)
{
    vec3 LightIntensity = Light.Color * Light.DiffuseIntensity;

    vec3 l = vec3(0.0);

    if (IsDirLight) {
        l = -PosDir.xyz;
    } else {
        l = PosDir - vertLocalPos;
        float LightToPixelDist = length(l);
        l = normalize(l);
        LightIntensity /= (LightToPixelDist * LightToPixelDist);
    }

    vec3 n = Normal;
    vec3 v = normalize(fragCameraPos - vertLocalPos);
    vec3 h = normalize(v + l);

    float nDotH = max(dot(n, h), 0.0);
    float vDotH = max(dot(v, h), 0.0);
    float nDotL = max(dot(n, l), 0.0);
    float nDotV = max(dot(n, v), 0.0);

    vec3 F = schlickFresnel(vDotH);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;

    vec3 SpecBRDF_nom  = ggxDistribution(nDotH) *
                         F *
                         geomSmith(nDotL) *
                         geomSmith(nDotV);

    float SpecBRDF_denom = 4.0 * nDotV * nDotL + 0.0001;

    vec3 SpecBRDF = SpecBRDF_nom / SpecBRDF_denom;

    vec3 fLambert = vec3(0.0);

    bool isMetal = vec3(texture(material.metallic, vertTexCoords)).r == 1.0;
    if (!isMetal)
    { fLambert = vec3(texture(material.diffuse, vertTexCoords)); }

    vec3 DiffuseBRDF = kD * fLambert / PI;

    vec3 FinalColor = (DiffuseBRDF + SpecBRDF) * LightIntensity * nDotL;

    return FinalColor;
}

void main()
{
    //fragColor = vec4(blinnPhongDir(vertNormal, vertFragPos, sunDir, lightColor), 1.0);

    BaseLight bl;
    bl.Color = lightColor;
    bl.AmbientIntensity = 0.5;
    bl.DiffuseIntensity = 0.5;
    fragColor = vec4(CalcPBRLighting(bl, sunDir, true, vertNormal), 1.0);

    vec3 ambient = vec3(ambientStrength) * vec3(texture(material.diffuse, vertTexCoords));
    fragColor += vec4(ambient, 1.0);

    // Gamma correction
    float gamma = 2.2;
    fragColor.rgb = pow(fragColor.rgb, vec3(1.0/gamma));
}