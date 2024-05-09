#version 330 core

out vec4 FragColor;

in vec3 TexCoord;
// in vec2 blockPos;

uniform sampler3D textureAtlas;

void main()
{
    FragColor = texture(textureAtlas, TexCoord);
}