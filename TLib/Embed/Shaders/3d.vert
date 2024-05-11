#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

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
uniform mat4 csmlightSpaceMatrices[shadowMapCascadeCount];
out vec4     csmLightClipPos[shadowMapCascadeCount];

void main()
{
    vertLocalPos          = position;
    vertWorldPos          = vec3(model * vec4(position, 1.0));
    vertNormal            = mat3(transpose(inverse(model))) * normal;
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