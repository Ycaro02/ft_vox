#ifndef VOX_HEADER_H
#define VOX_HEADER_H

#include <math.h>					            /* LIBC Math functions */
#include "glad/gl.h"				            /* Glad functions (Include glad header BEFORE glfw3) */
#include "glfw3/glfw3.h"			            /* GLFW functions */

#include "../rsc/deps/cglm/include/cglm/cglm.h" 

#include "../libft/libft.h"		                /* Libft functions */
#include "../libft/parse_flag/parse_flag.h"		/* Parse flag functions */
#include "../libft/hashMap/hashMap.h"			/* Hashmap functions */
#include "../libft/BMP_parser/parse_bmp.h"		/* BMP parser functions */
#include "window.h"					            /* Window related functions */
#include "win_event.h"				            /* Window event handling functions */
#include "camera.h"                             /* Camera handling function */
#include "render.h"								/* Render functions */
#include "chunks.h"								/* Chunks functions */


/* Screen size */
#define SCREEN_WIDTH 1920			/* Screen width */
#define SCREEN_HEIGHT 1080			/* Screen height */

/* Texture atlas path */
#define TEXTURE_ATLAS_PATH "rsc/texture/texture_atlas.bmp"

/* Block hiden deine */
#define BLOCK_HIDDEN 1U

/* World Max size */
#define WORLD_MAX_HEIGHT 256U
#define WORLD_MAX_WIDTH (16384U * 2U)
#define WORLD_MAX_DEPTH (16384U * 2U)


/* Shader path */
#define VERTEX_SHADER_PATH		"rsc/shaders/vertex_shader.glsl"
#define FRAGMENT_SHADER_PATH	"rsc/shaders/fragment_shader.glsl"




/* HARDCOED CHUNK NUMBER */
#define TEST_CHUNK_MAX 9U

enum block_type {
    AIR=0U,      /* Air block (Empty) */
    DIRT=1U,     /* Dirt block (Exposed to light) */
    STONE=2U,    /* Stone block (Underground) */
};

typedef struct PACKED_STRUCT s_block {
    s32 x;          /* Block x position */
    s32 y;          /* Block y position (height) */
    s32 z;          /* Block z position */
    u32 type;       /* Block type */
	u8  flag;       /* Block flag */
    // u32 chunkId;    /* Chunk ID */
}   t_block;


typedef struct s_world {
	u64			seed;			/* World seed */
	t_chunks	*chunks;		/* Chunks array */
	u32			nb_chunks;		/* Number of chunks loaded */
} t_world;

/* Context structure */
typedef struct s_context {
	t_camera	cam;			/* camera structure */
    GLFWwindow	*win_ptr;		/* Window pointer */
	t_modelCube	cube;			/* Data Cube structure */
    t_chunks    *chunks;        /* current chunk */
	GLuint		shader_id;		/* shader program id */
	u32			renderBlock;	/* Total block to render */
} t_context;

/* Atlas texture ID */
enum AtlasId {
	ATLAS_DIRT_PINK=0, /* Pink to remove */
	ATLAS_SAND=1,
	ATLAS_STONE_CUT=2,
	ATLAS_BRICK=3,
	ATLAS_WOOD=4,
	ATLAS_STONE=5,
	ATLAS_DIRT=6,
	ATLAS_WOOD_PLANK=7, /* same here */
	ATLAS_DIRT2=8,
	ATLAS_GLASS=9,
	ATLAS_COBBLESTONE=10,
	ATLAS_FULL_GREY=11,
	ATLAS_STONE_CLEAN=12,
};


/* render/cube.c */
GLuint	setupCubeVAO(t_context *c, t_modelCube *cube);
void	drawAllCube(GLuint vao, u32 nb_cube);

/* texture load_texture */
GLuint *load_texture_atlas();
void set_shader_texture(t_context *c, GLuint *atlas, u32 index);

/* render/occlusion_culling */
u32 checkHiddenBlock(t_chunks *chunks, u32 subChunksID);

#endif /* VOX_HEADER_H */


// struct compact_block {
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
