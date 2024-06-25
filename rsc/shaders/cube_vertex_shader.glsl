#version 330 core


/* Define the block faces */
#define TOP_FACE 5

/* Define the biomes */
#define PLAIN_BIOME 0

/* Define the input variables */
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aInstancePos;
layout (location = 2) in vec2 aTexCoord;
/**
 * aMetadata is a 32 bit integer that contains the following information:
 * byte 3: texture ID 
 * byte 2: block face
 * byte 1: biome ID
 * byte 0: light level	--> to implement
*/
layout (location = 3) in int aMetadata;

/* Define the output variables */
out vec3 TexCoord;
flat out int biomeType;
flat out int isGrass;
flat out int isTopFace;

/* Define the uniforms variables */
uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

/* Define the texture atlas */
uniform int GRASS_SIDE;
uniform int GRASS_TOP;

uniform int SNOW_SIDE;
uniform int SNOW_TOP;

uniform int SANDSTONE_SIDE;
uniform int SANDSTONE_TOP;
uniform int ATLAS_SIZE;


int topBlockFaceHandling(int textureID, int blockFace) {
	
	if (blockFace == TOP_FACE) {
		isTopFace = 1;
		if (textureID == GRASS_SIDE) {
			return (GRASS_TOP);
		} else if (textureID == SNOW_SIDE) {
			return (SNOW_TOP);
		} else if (textureID == SANDSTONE_SIDE) {
			return (SANDSTONE_TOP);
		}
	}
	return (textureID);
}

int s32ByteGet(int value, int byte) {
	return (value & (0xFF << (byte * 8))) >> (byte * 8);
}

void main()
{
    int textureIdExtracted = s32ByteGet(aMetadata, 3);
    int blockFace = s32ByteGet(aMetadata, 2);
	
	biomeType = s32ByteGet(aMetadata, 1);
	isTopFace = 0;

	if (textureIdExtracted == GRASS_SIDE) {
		isGrass = 1;
	}

	int realTextureId = topBlockFaceHandling(textureIdExtracted, blockFace);

	TexCoord = vec3(aTexCoord, float(realTextureId) / float(ATLAS_SIZE));
    mat4 m = model;
    m[3] = vec4(aInstancePos, 1.0) + model[3];
    vec4 worldPosition = m * vec4(aPos, 1.0);
    gl_Position = projection * view * worldPosition;
}