#ifndef VOX_HEADER_H
#define VOX_HEADER_H

#include <math.h>					            /* LIBC Math functions */
#include "glad/gl.h"				            /* Glad functions (Include glad header BEFORE glfw3) */
#include "glfw3/glfw3.h"			            /* GLFW functions */
#include "../rsc/deps/cglm/include/cglm/cglm.h" /* CGLM C maths openGL */
#include "../src/tinycthread/tinycthread.h"        /* TinyCThread functions */

#include "../libft/libft.h"		                /* Libft functions */
#include "../libft/parse_flag/parse_flag.h"		/* Parse flag functions */
#include "../libft/HashMap/HashMap.h"			/* Hashmap functions */
#include "../libft/BMP_parser/parse_bmp.h"		/* BMP parser functions */
#include "window.h"					            /* Window related functions */
#include "win_event.h"				            /* Window event handling functions */
#include "camera.h"                             /* Camera handling function */
#include "shader_utils.h"						/* Shader utils functions */

/* Screen size */
#define SCREEN_WIDTH	1920			/* Screen width */
#define SCREEN_HEIGHT	1080			/* Screen height */

#define CAM_FOV			80.0f			/* Camera field of view */
#define CHUNK_FLOAT_SIZE 8.0f			/* Chunk float size */
#define ANGLE_INCREMENT 5.0f			/* Angle increment in chunk veiw handling */

/* Basic value */
#define CHUNKS_LOAD_RADIUS 15							/* Chunks load radius */
#define CHUNKS_UNLOAD_MAX (CHUNKS_LOAD_RADIUS + 6)		/* Chunks unload max radius */
#define MAX_RENDER_DISTANCE (CHUNK_FLOAT_SIZE * 10.0f)	/* Max render distance */

/* DEBUG VAL */
// #define CHUNKS_LOAD_RADIUS 1	
// #define CHUNKS_UNLOAD_MAX (CHUNKS_LOAD_RADIUS + 5)
// #define MAX_RENDER_DISTANCE (CHUNK_FLOAT_SIZE * 2.0f)



#define TRAVEL_INCREMENT CHUNK_FLOAT_SIZE							/* Travel increment */

/* Texture atlas path */
#define TEXTURE_ATLAS_PATH "rsc/texture/atlas.bmp"

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


#define DIR_RIGHT 0
#define DIR_LEFT 1

/* HARDCOED CHUNK NUMBER */
#define TEST_CHUNK_MAX 9

/* Perlin noise value generation for relief */
#define PERLIN_OCTAVE 8
#define PERLIN_PERSISTENCE 1.0
#define PERLIN_LACUNARITY 2.0
// #define PERLIN_OCTAVE 6
// #define PERLIN_PERSISTENCE 1.0
// #define PERLIN_LACUNARITY 2.0

#define SEA_LEVEL 80.0f

#define CHUNK_FRONT 0
#define CHUNK_BACK 1
#define CHUNK_RIGHT 2
#define CHUNK_LEFT 3

/* Neighbor flag value */
#define NEIGHBOR_BACK  	1U
#define NEIGHBOR_FRONT  2U
#define NEIGHBOR_LEFT   4U
#define NEIGHBOR_RIGHT 	8U
#define NEIGHBOR_BOTTOM 16U
#define NEIGHBOR_TOP 	32U

#define CHUNKS_NEIGHBOR_LOADED (NEIGHBOR_BACK | NEIGHBOR_FRONT | NEIGHBOR_LEFT | NEIGHBOR_RIGHT)

/* If all neighnor block is hidden */
#define BLOCK_HIDDEN (NEIGHBOR_RIGHT | NEIGHBOR_LEFT | NEIGHBOR_TOP | NEIGHBOR_BOTTOM | NEIGHBOR_FRONT | NEIGHBOR_BACK)

typedef struct PACKED_STRUCT s_block {
    s8 	x;          /* Block x position */
    s8 	y;          /* Block y position (height) */
    s8 	z;          /* Block z position */
    s8	type;       /* Block type */
	u8  neighbors;  /* Block flag */
}   Block;

enum AtlasID {
    AT_BREAK_STONE=0,     /* Break stone */
    AT_BREAK_STONE1,        /* Break stone 1 */
    AT_BREAK_STONE2,        /* Break stone 2 */
    AT_BREAK_STONE3,        /* Break stone 3 */
    AT_BREAK_STONE4,        /* Break stone 4 */
    AT_BREAK_STONE5,        /* Break stone 5 */
    AT_BREAK_STONE6,        /* Break stone 6 */
    AT_BREAK_STONE7,        /* Break stone 7 */
    AT_BREAK_STONE8,        /* Break stone 8 */
    AT_BREAK_STONE9,        /* Break stone 9 */
    AT_WOOL_RED=10,            /* Wool red */
    AT_WOOL_ORANGE,         /* Wool orange */
    AT_WOOL_YELLOW,         /* Wool yellow */
    AT_WOOL_LIGHTGREEN,     /* Wool lime */
    AT_WOOL_GREEN,          /* Wool green */
    AT_WOOL_CYAN,           /* Wool cyan */
    AT_WOOL_LIGHT_BLUE,     /* Wool light blue */
    AT_WOOL_BLUE,           /* Wool blue */
    AT_WOOL_PURPLE,         /* Wool purple */
    AT_WOOL_MAGENTA,        /* Wool magenta */
    AT_WOOL_LIGHTMAGENTA,   /* Wool magenta */
    AT_WOOL_PINK,           /* Wool pink */
    AT_WOOL_DARKPINK,       /* Wool pink */
    AT_WOOL_GRAY,           /* Wool gray */
    AT_WOOL_LIGHT_GRAY,     /* Wool light gray */
    AT_WOOL_WHITE,          /* Wool white */
    AT_SPONGE=26,           /* Sponge */
    AT_GLASS,               /* Glass */
	AT_EMPTYTOREMOVE=28,	/* Empty to remove */
    AT_FULL_WHITE_START,    /* Full white start */
    AT_FULL_GOLD_START,     /* Full gold start */
    AT_GOLD_ORE=31,         /* Gold ore */
    AT_IRON_ORE,            /* Iron ore */
    AT_COAL_ORE,            /* Coal ore */
    AT_BOOKSHELF,           /* Bookshelf */
    AT_STONE_WITH_VINES,    /* Stone with vines */
    AT_OBSIDIAN,            /* Obsidian */
    AT_SEMI_FULL_WHITE,     /* Semi full white */
    AT_SEMI_FULL_GOLD,      /* Semi full gold */
    AT_COBBLESTONE=39,      /* Cobblestone */
    AT_BEDROCK,             /* Bedrock */
    AT_SAND,                /* Sand */
    AT_GRAVEL,              /* Gravel */
    AT_LOG_SIDE,            /* Log side */
    AT_LOG_INTERIOR,        /* Log interior */
    AT_LEAVES,				/* Leaves */
    AT_FULL_WHITE=46,       /* Full white */
	AT_FULLGOLD=47,			/* Full gold */
	AT_SHROOM_REDW=48,		/* Red White mushroom */
	AT_SHROOM_BROWN=49,		/* Brown mushroom */
	AT_LAVE=50,				/* Lava */
	AT_GRASS_TOP=51,		/* Grass top */
	AT_GRASS_TOP2=52,		/* Grass top */
	AT_STONE=53,			/* Stone */
	AT_DIRT=54,				/* Dirt */
	AT_GRASS_SIDE=55,		/* Grass side */
	AT_WOOD_PLANK=56,		/* Wood plank */
	AT_STONE_CUT=57,		/* Stone cut */
	AT_FULLGREY=58,			/* Full grey */
	AT_BRICK=59,			/* Brick */
	AT_TNT_SIDE=60,			/* TNT side */
	AT_TNT_TOP=61,			/* TNT top */
	AT_TNT_BOTTOM=62,		/* TNT bottom */
	AT_SPIDERWEB=63,		/* Spider web */
	AT_ROSE=64,				/* Rose */
	AT_SUNFLOWER=65,		/* Sunflower */
	AT_WATER=66,			/* Water */
	AT_LITTLE_TREE=67,		/* Little tree */
};

enum BlockType {
    AIR=0,      /* Air block (Empty) */
    STONE=53,    /* Stone block (Underground) */
    DIRT=54,     /* Dirt block (Exposed to light) */
	GRASS=55,    /* Grass block (Exposed to light) */
	GRASS_TOP=AT_GRASS_TOP,	/* Grass top */
	WATER=AT_WATER,		/* Water */
};

/* texture load_texture */
GLuint load_texture_atlas(char *path, int squareHeight, int squareWidth);
void set_shader_texture(GLuint shaderId, GLuint atlasID, u32 textureType, char *varName);
GLuint load_cubemap(char* path, int squareHeight, int squareWidth);


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


// struct compacBlock {
// 	u32 shape_texture;
// 	/*
// 		u16 shape;
// 		u16 texture;
// 	*/
// 	u32 y_light;
// 	/*
// 		u8 flag;
// 		u8 torch_sun_light;  4 bits for torch light and 4 bits for sun light
// 		u16 y;
// 	*/
// 	u32 x;		/* x position */
// 	u32 z;		/* z position */
// };
