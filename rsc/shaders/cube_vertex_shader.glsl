#version 330 core

#define GRASS_SIDE 55.0
#define GRASS_TOP 54.0
#define SNOW_SIDE 100.0
#define SNOW_TOP 99.0

#define TOP_FACE 5.0
#define ATLAS_SIZE 101.0

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

    
	if (aTextureID == GRASS_SIDE && blockFace == TOP_FACE) {
		textureID = GRASS_TOP;
	} else if (aTextureID == SNOW_SIDE && blockFace == TOP_FACE) {
		textureID = SNOW_TOP;
	}

	TexCoord = vec3(realTexCoord, textureID / ATLAS_SIZE);

    mat4 m = model;
    m[3] = vec4(aInstancePos, 1.0) + model[3];
    vec4 worldPosition = m * vec4(aPos, 1.0);
    gl_Position = projection * view * worldPosition;
}