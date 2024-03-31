#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out vec3 vertNormal;
out vec2 vertTexCoords;
out vec3 vertFragPos;
out vec3 vertLightPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 lightPos;

void main()
{
    gl_Position   = projection * view * model * vec4(position, 1.0);

    // https://learnopengl.com/Lighting/Basic-Lighting
    vertNormal    = mat3(transpose(inverse(model))) * normal;

    vertTexCoords = texCoords;
    vertFragPos   = vec3(model * vec4(position, 1.0));
    vertLightPos  = vec3(view * vec4(lightPos, 1.0));
}