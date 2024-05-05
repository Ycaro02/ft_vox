#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aInstancePos;
layout (location = 2) in vec2 aTexCoord;

out vec2 TexCoord;

// out vec2 blockPos;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

void main()
{
	TexCoord = aTexCoord;
    // blockPos = vec2(aInstancePos.x, aInstancePos.z);
    mat4 m = model;
	m[3] = vec4(aInstancePos, 1.0) + model[3];
    gl_Position = projection * view * m * vec4(aPos, 1.0);
}
