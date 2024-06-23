#version 330 core

// #define GRASS_SIDE 55.0
// #define GRASS_TOP 54.0

#define GRASS_SIDE 97
#define GRASS_TOP 96

#define PLAIN_BIOME 0

#define SNOW_SIDE 104
#define SNOW_TOP 103

#define SANDSTONE_SIDE 98
#define SANDSTONE_TOP 99

#define TOP_FACE 5.0
// #define ATLAS_SIZE 101.0
#define ATLAS_SIZE 105

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aInstancePos;
layout (location = 2) in vec3 aTexCoord;
layout (location = 3) in int aTextureID;

out vec3 TexCoord;

flat out int biomeType;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

void main()
{
    vec2 realTexCoord = aTexCoord.xy;
    float blockFace = aTexCoord.z;
    // float textureID = float(aTextureID);
    int textureID = aTextureID;

	biomeType = 99; /* To implementent give it at uniform value */

	if (aTextureID == GRASS_SIDE) {
		biomeType = PLAIN_BIOME;
	}

	if (blockFace == TOP_FACE) {
		if (aTextureID == GRASS_SIDE) {
			textureID = GRASS_TOP;
		} else if (aTextureID == SNOW_SIDE) {
			textureID = SNOW_TOP;
		} else if (aTextureID == SANDSTONE_SIDE) {
			textureID = SANDSTONE_TOP;
		}
	}

	TexCoord = vec3(realTexCoord, float(textureID) / float(ATLAS_SIZE));
    mat4 m = model;
    m[3] = vec4(aInstancePos, 1.0) + model[3];
    vec4 worldPosition = m * vec4(aPos, 1.0);
    gl_Position = projection * view * worldPosition;
}