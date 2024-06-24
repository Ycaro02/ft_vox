#ifndef VOX_HEADER_H
#define VOX_HEADER_H

#include "typedef_struct.h"		                /* Struct typedef */
#include "block_type.h"			                /* Block type enum */

/* Screen size */
#define SCREEN_WIDTH		1920			/* Screen width */
#define SCREEN_HEIGHT		1080			/* Screen height */

#define CAM_FOV				80.0f			/* Camera field of view */
#define CHUNK_FLOAT_SIZE	8.0f			/* Chunk float size, cause one block is 0.5, 0.5 * 16 = 8.0f */
#define ANGLE_INCREMENT		3.0f			/* Angle increment in chunk veiw handling */
#define TRAVEL_INCREMENT 	3.0f				/* Travel increment */

/* Basic value */
#define CHUNKS_LOAD_RADIUS 16							/* Chunks load radius */
#define CHUNKS_UNLOAD_RADIUS (CHUNKS_LOAD_RADIUS * 2)	/* Chunks unload max radius */
#define MAX_RENDER_DISTANCE (CHUNK_FLOAT_SIZE * 16.0f)	/* Max render distance */

#define PERLIN_BIOME_SCALE 4.0f

/* Bonus Value */
// #define CHUNKS_LOAD_RADIUS 25								/* Chunks load radius */
// #define CHUNKS_UNLOAD_RADIUS (CHUNKS_LOAD_RADIUS + 10)		/* Chunks unload max radius */
// #define MAX_RENDER_DISTANCE (CHUNK_FLOAT_SIZE * 16.0f)		/* Max render distance */

/* DEBUG VAL */
// #define CHUNKS_LOAD_RADIUS 1	
// #define CHUNKS_UNLOAD_RADIUS (CHUNKS_LOAD_RADIUS + 5)
// #define MAX_RENDER_DISTANCE (CHUNK_FLOAT_SIZE * 2.0f)



/* Texture atlas path size 105 */
#define TEXTURE_ATLAS_PATH "rsc/texture/atlas_block.bmp"

/* Texture atlas path size 105 */
// #define TEXTURE_ATLAS_PATH "rsc/texture/test_atlas.bmp"

/*TEST for rebuild atlas*/
// #define TEXTURE_ATLAS_PATH "rsc/texture/TESTatlas_block.bmp"

/* Sky path */
#define TEXTURE_SKY_PATH "rsc/texture/skybox.bmp"

/* World Max size */
#define WORLD_MAX_HEIGHT 256U
#define WORLD_MAX_WIDTH (16384U * 2U)
#define WORLD_MAX_DEPTH (16384U * 2U)


/* Shader path */
#define CUBE_VERTEX_SHADER		"rsc/shaders/cube_vertex_shader.glsl"
#define CUBE_FRAGMENT_SHADER	"rsc/shaders/cube_fragment_shader.glsl"

#define SKY_VERTEX_SHADER		"rsc/shaders/sky_vertex_shader.glsl"
#define SKY_FRAGMENT_SHADER		"rsc/shaders/sky_fragment_shader.glsl"

#define CHAR_VERTEX_SHADER		"rsc/shaders/char_vertex_shader.glsl"
#define CHAR_FRAGMENT_SHADER	"rsc/shaders/char_fragment_shader.glsl"


/* Perlin noise value generation for relief */
#define PERLIN_OCTAVE 8
#define PERLIN_PERSISTENCE 1.0
#define PERLIN_LACUNARITY 2.0

/* Perline noise snake val generation */

#define PERLIN_SNAKE_HEIGHT 2048
#define PERLIN_SNAKE_WIDTH 2048

#define PERLIN_SNAKE_OCTAVE 6
#define PERLIN_SNAKE_PERSISTENCE 0.6
#define PERLIN_SNAKE_LACUNARITY 2.0

// https://dawnosaur.substack.com/p/how-minecraft-generates-worlds-you Explain how minecraft generate world with different noise
/* Perlin generation value for continental noise */
#define PERLIN_CONTINENTAL_OCTAVE 8
#define PERLIN_CONTINENTAL_PERSISTENCE 0.6f
#define PERLIN_CONTINENTAL_LACUNARITY 2.0f
/* Perlin generation value for erosion noise */
#define PERLIN_EROSION_OCTAVE 8
#define PERLIN_EROSION_PERSISTENCE 0.6f
#define PERLIN_EROSION_LACUNARITY 1.8f
/* Perlin generation value for picks and valley noise */
#define PERLIN_PICKS_VALLEY_OCTAVE 10
#define PERLIN_PICKS_VALLEY_PERSISTENCE 0.5f
#define PERLIN_PICKS_VALLEY_LACUNARITY 3.0f
/* second test value for peaks and valley */
// #define PERLIN_PICKS_VALLEY_OCTAVE 6
// #define PERLIN_PICKS_VALLEY_PERSISTENCE 0.6f
// #define PERLIN_PICKS_VALLEY_LACUNARITY 2.2f
// /* Perlin generation value for humidity noise */
#define PERLIN_HUMIDITY_OCTAVE 4
#define PERLIN_HUMIDITY_PERSISTENCE 1.0f
#define PERLIN_HUMIDITY_LACUNARITY 2.0f
// /* Perlin generation value for temperature noise */
#define PERLIN_TEMPERATURE_OCTAVE 6
#define PERLIN_TEMPERATURE_PERSISTENCE 0.4f
#define PERLIN_TEMPERATURE_LACUNARITY 2.0f

/* Chunk generation */
#define MIN_HEIGHT 80.0f
#define SEA_LEVEL 60

/* Chunk neighbor idx */
#define CHUNK_FRONT 0
#define CHUNK_BACK 1
#define CHUNK_RIGHT 2
#define CHUNK_LEFT 3

enum BlockType {
    AIR=0,									/* Air block (Empty) */
    STONE=NEWAT_STONE,						/* Stone block (Underground) */
    DIRT=NEWAT_DIRT,						/* Dirt block */
	GRASS=NEWAT_GREY_GRASS_SIDE,			/* Grass block */
	GRASS_TOP=NEWAT_GREY_GRASS_TOP,			/* Grass top */
	WATER=NEWAT_WATER,						/* Water */
	SAND=NEWAT_SAND,						/* Sand */
	BEDROCK=NEWAT_BEDROCK,					/* Bedrock */
	SNOW=NEWAT_SNOW,						/* Snow */
	ICE=NEWAT_ICE,							/* Ice */
	SNOW_GRASS=NEWAT_SNOW_GRASS_SIDE,		/* Snow grass */
	SANDSTONE=NEWAT_SANDSTONE_SIDE,			/* Sandstone */
	SANDSTONE_TOP=NEWAT_SANDSTONE_TOP,		/* Sandstone top */
	STONE_BRICK=NEWAT_STONE_BRICK,			/* Stone brick --> Unused */
	GLASS=NEWAT_GLASS,						/* Glass --> Unused */
};

/* texture load_texture */
GLuint	load_texture_atlas(char *path, int squareHeight, int squareWidth);
void	set_shader_texture(GLuint shaderId, GLuint atlasID, u32 textureType, char *varName);
GLuint	load_cubemap(char* path, int squareHeight, int squareWidth);


#include <sys/time.h>

/**
 * @brief Get current time in microseconds
 * @return current time in microseconds
*/
FT_INLINE suseconds_t  get_ms_time(void) {
	struct timeval  now;

	if (gettimeofday(&now, NULL) != 0)
			return (0);
	return ((now.tv_sec * 1000000) + now.tv_usec);
}

#endif /* VOX_HEADER_H */