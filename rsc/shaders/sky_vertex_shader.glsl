#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in float aFaceID;

out vec3 TexCoords;
out float FaceID;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aPos;
	FaceID = aFaceID;
	
    vec4 pos = projection * view * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}