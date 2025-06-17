
#version 330 core
in  vec2 fragTextureUV;
in  vec4 fragColor;
out vec4 outColor;

uniform sampler2D image;

void main()
{
    outColor = fragColor * texture(image, fragTextureUV);
}