
#version 330 core
in vec2 fragTexCoords;
in vec4 fragColor;
out vec4 outColor;

uniform sampler2D image;

uniform float width = 0.45;
uniform float edge  = 0.1;

void main()
{
    float distance = texture(image, fragTexCoords).r;
    float alpha    = smoothstep(width, width + edge, distance);

    outColor = vec4(fragColor.xyz, alpha);
}