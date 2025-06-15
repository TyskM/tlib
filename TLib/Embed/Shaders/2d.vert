#version 330 core
layout (location = 0) in vec2 vertex;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 textureUV;

out vec2 fragTextureUV;
out vec4 fragColor;

uniform mat4 projection;

void main()
{
    gl_Position   = projection * vec4(vertex, 0.0, 1.0);
    fragTextureUV = textureUV;
    fragColor     = color;
}
