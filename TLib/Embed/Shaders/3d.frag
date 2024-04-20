#version 330 core
out vec4 fragColor;

in vec3 vertNormal;
in vec2 vertTexCoords;
in vec3 vertFragPos;
in vec3 vertLightPos;

uniform vec3  viewPos;
uniform vec3  lightColor       = vec3(0.2, 0.2, 0.2);
uniform float ambientStrength  = 0.1;
uniform float specularStrength = 0.7;
uniform vec3  sunDir           = vec3(-0.2, -1.0, -0.3);

//uniform sampler2D texture_diffuse;
//uniform sampler2D texture_specular;

struct Material
{
    sampler2D diffuse;
    sampler2D specular;
    float     shininess;
};

uniform Material material;

//vec3 getLightingFromDir(vec3 lightReceiveDir)
//{
//    vec3 lightDir = normalize(-lightReceiveDir);
//
//    // Ambient Lighting
//    vec3 lightAmbient = (vec3(0.2) * lightColor) * vec3(texture(material.diffuse, vertTexCoords));
//
//    // Diffuse lighting
//    vec3  norm         = normalize(vertNormal);
//    float diff         = max(dot(norm, lightDir), 0.0);
//    vec3  lightDiffuse = (vec3(0.5) * lightColor) * diff * vec3(texture(material.diffuse, vertTexCoords));
//
//    // Specular
//    vec3  viewDir    = normalize(-vertFragPos);
//    vec3  reflectDir = reflect(-lightDir, norm);  
//    float spec       = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
//    vec3  specular   = lightColor * spec * vec3(texture(material.specular, vertTexCoords));
//
//    vec3 lightResult = lightAmbient + lightDiffuse + specular;
//    return lightResult;
//}
//
//vec3 getLightingFromPoint(vec3 point)
//{
//    vec3 lightDir = normalize(point - vertFragPos);
//    return getLightingFromDir(-lightDir);
//}

vec3 blinnPhongDir(vec3 normal, vec3 fragPos, vec3 lightDirRaw, vec3 lightColor)
{
    // ambient
    vec3 ambient = lightColor * vec3(texture(material.diffuse, vertTexCoords));;

    // diffuse
    vec3  lightDir = normalize(-lightDirRaw);
    float diff     = max(dot(lightDir, normal), 0.0);
    vec3  diffuse  = diff * lightColor * vec3(texture(material.diffuse, vertTexCoords));

    // specular
    vec3 viewDir    = normalize(viewPos - fragPos);
    float spec      = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec            = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    vec3 specular   = spec * lightColor * vec3(texture(material.specular, vertTexCoords)); 
   
    return ambient + diffuse + specular;

    // simple attenuation
    // float max_distance = 1.5;
    // float distance     = length(lightPos - fragPos);
    // float attenuation  = 1.0 / (distance * distance);
    //diffuse  *= attenuation;
    //specular *= attenuation;
}


void main()
{
    //vec4 texColor = texture(texture_diffuse, vec2(vertTexCoords.x, vertTexCoords.y));
    //fragColor = vec4(getLightingFromPoint(vertLightPos), 1.0) * texColor;
    //fragColor = vec4(getLightingFromDir(sunDir), 1.0) * texColor;
    fragColor = vec4(blinnPhongDir(vertNormal, vertFragPos, sunDir, lightColor), 1.0);

    // Gamma correction
    float gamma = 2.2;
    fragColor.rgb = pow(fragColor.rgb, vec3(1.0/gamma));
}