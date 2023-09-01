
#version 330 core
layout (location = 0) in vec4 vertex;
layout (location = 1) in vec4 color;

out vec2 fragTexCoords;
out vec4 fragColor;

uniform mat4 projection;

void main()
{
    gl_Position   = projection * vec4(vertex.xy, 0.0, 1.0);
    fragTexCoords = vertex.zw;
    fragColor     = color;
}
