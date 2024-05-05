#version 330 core

out vec4 FragColor;

in vec2 TexCoord;
in vec2 blockPos;

uniform sampler2D texture1;

void main()
{
    if ((blockPos.x > 0.0 && blockPos.x < 16.0) && (blockPos.y > 0.0 && blockPos.y < 16.0)) {
        FragColor = vec4(1.0, 0.0, 0.0, 1.0);
        return ;
    } else if ((blockPos.x > 16.0 && blockPos.x < 32.0) && (blockPos.y > 16.0 && blockPos.y < 32.0)) {
        FragColor = vec4(0.0, 1.0, 0.0, 1.0);
        return ;
    } else if ((blockPos.x > -16.0 && blockPos.x < 0.0) && (blockPos.y > -16.0 && blockPos.y < 0.0)) {
        FragColor = vec4(0.0, 0.0, 1.0, 1.0);
        return ;
    }
    FragColor = texture(texture1, TexCoord);
}