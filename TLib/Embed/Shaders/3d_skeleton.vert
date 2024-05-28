#version 330 core
layout (location = 0) in vec3  position;
layout (location = 1) in vec3  normal;
layout (location = 2) in vec2  texCoords;
layout (location = 3) in ivec4 boneIds; 
layout (location = 4) in vec4  weights;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

out vec3 vertLocalPos;
out vec3 vertWorldPos;
out vec3 vertNormal;
out vec2 vertTexCoords;
out vec4 vertFragPosLightSpace;
out vec3 vertClipPos;

// Inject //! #define shadowMapCascadeCount 3
uniform mat4 csmlightSpaceMatrices [shadowMapCascadeCount];
out vec4     csmLightClipPos       [shadowMapCascadeCount];

// Inject //! #define maxBoneInfluences 4
// Inject //! #define maxBones          100
uniform mat4 boneMatrices[maxBones];

void main()
{
    mat4 boneTransform = boneMatrices[boneIds[0]] * weights[0];
    boneTransform     += boneMatrices[boneIds[1]] * weights[1];
    boneTransform     += boneMatrices[boneIds[2]] * weights[2];
    boneTransform     += boneMatrices[boneIds[3]] * weights[3];

    vec4 bonePosLocal    = boneTransform * vec4(position, 1.0);
    vec4 boneNormalLocal = boneTransform * vec4(normal, 1.0);

    vertLocalPos          = vec3(bonePosLocal);
    vertWorldPos          = vec3(model * vec4(vertLocalPos, 1.0));
    vertNormal            = vec3(mat4(transpose(inverse(model))) * boneNormalLocal);
    vertTexCoords         = texCoords;
    vertFragPosLightSpace = lightSpaceMatrix * vec4(vertWorldPos, 1.0);
    gl_Position = projection * view * vec4(vertWorldPos, 1.0);
    vertClipPos = gl_Position.xyz;

    for (int i = 0; i < shadowMapCascadeCount; i++)
    {
        csmLightClipPos[i] =
        csmlightSpaceMatrices[i] * vec4(vertWorldPos, 1.0);
    }
}