#version 330 core

#define BIOME_PLAIN 0
#define BIOME_JUNGLE 3
#define BIOME_SWAMP 4
#define PLAIN_GRASS_COLOR vec3(0.9, 1.0, 0.0)
#define JUNGLE_GRASS_COLOR vec3(0.5, 1.0, 0.5)
#define SWAMP_GRASS_COLOR vec3(0.0, 0.5, 0.5)

/* Define the block faces */
#define BACK_FACE	0
#define FRONT_FACE	1
#define LEFT_FACE	2
#define RIGHT_FACE	3
#define BOTTOM_FACE 4
#define TOP_FACE	5

#define SCREEN_WIDTH 1920

out vec4 FragColor;

in vec3 TexCoord;
flat in int biomeType;
flat in int isGrass;
flat in int blockFace;
flat in int isFlower;

#define CAMERA_IN_CAVE 2

uniform int camIsUnderground;
uniform sampler3D textureAtlas;

/* render distance is 16 and we need minimum 14, one chunks is 16.0 len, so we end fog at 16.0 * 14 = 224.0 */

#define CHUNKS_SIZE 16.0

// #define RENDER_DISTANCE_MAX 14.0
#define RENDER_DISTANCE_MAX 12.0

#define START_FOG (CHUNKS_SIZE * (RENDER_DISTANCE_MAX - 2.0))
#define END_FOG (CHUNKS_SIZE * RENDER_DISTANCE_MAX)

const float grayTolerance = 0.01;
float fogStart = START_FOG;
float fogEnd = END_FOG; 
vec4 fogColor = vec4(1.0, 1.0, 1.0, 0.0);

vec4 grassColorHandling(vec4 baseColor) {
	vec3 grassColor = PLAIN_GRASS_COLOR; /* Plain */
	if (biomeType == BIOME_JUNGLE) {
		grassColor = JUNGLE_GRASS_COLOR; /* Jungle */
	} else if (biomeType == BIOME_SWAMP) {
		grassColor = SWAMP_GRASS_COLOR; /* Swamp */
	}
	return (vec4(baseColor.rgb * grassColor, baseColor.a));
}

void main()
{
    vec4 baseColor = texture(textureAtlas, TexCoord);
	vec4 finalColor = baseColor;
    bool isGray = abs(baseColor.r - baseColor.g) < grayTolerance && abs(baseColor.g - baseColor.b) < grayTolerance && abs(baseColor.r - baseColor.b) < grayTolerance;

	// if (isFlower == 1 && (blockFace != RIGHT_FACE)) {
	if (isFlower == 1 && (blockFace != FRONT_FACE && blockFace != RIGHT_FACE)) {
		discard ;
	}

	float depth = gl_FragCoord.z / gl_FragCoord.w;
	float fogFactor = smoothstep(fogStart, fogEnd, depth);

	if (isGray && isGrass == 1) {
		baseColor = grassColorHandling(baseColor);
	}

	if (isFlower == 0) {
		/* Reduce light if not top face */
		if (blockFace != TOP_FACE) {
			if (blockFace == BOTTOM_FACE) {
				baseColor = vec4(baseColor.rgb * 0.85, baseColor.a);

			} else {
				baseColor = vec4(baseColor.rgb * 0.9, baseColor.a);
			}
		}
		if (camIsUnderground == CAMERA_IN_CAVE || camIsUnderground == 1) {
			fogStart = 40.0;
			fogEnd = 120.0;
			fogFactor = smoothstep(fogStart, fogEnd, depth);
			fogColor = vec4(0.0,0.0,0.0, 1.0);
		} else {
			fogColor = vec4(baseColor.xyz, 0.0);
		}
		finalColor = mix(baseColor, fogColor, fogFactor);
	} 

	if (isFlower == 1 && fogFactor > 0.2) {
		discard;
	}


    FragColor = finalColor;
}