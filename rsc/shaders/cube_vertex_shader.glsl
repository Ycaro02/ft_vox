#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aInstancePos;
layout (location = 2) in vec3 aTexCoord;
layout (location = 3) in float aTextureID;

out vec3 TexCoord;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

void main()
{
    vec2 realTexCoord = aTexCoord.xy;
    float blockFace = aTexCoord.z;
    float textureID = aTextureID;

    if (aTextureID == 55.0 && blockFace == 5.0) {
        textureID = 51.0;
    }

    TexCoord = vec3(realTexCoord, textureID / 67.0);
    

    mat4 m = model;
    m[3] = vec4(aInstancePos, 1.0) + model[3];
    vec4 worldPosition = m * vec4(aPos, 1.0);
    gl_Position = projection * view * worldPosition;
}
