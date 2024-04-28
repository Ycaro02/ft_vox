#ifndef VOX_HEADER_H
#define VOX_HEADER_H

#include <math.h>					            /* LIBC Math functions */
#include "glad/gl.h"				            /* Glad functions (Include glad header BEFORE glfw3) */
#include "glfw3/glfw3.h"			            /* GLFW functions */
#include "../libft/libft.h"		                /* Libft functions */
#include "../libft/parse_flag/parse_flag.h"    /* Parse flag functions */
#include "window.h"					            /* Window related functions */
#include "win_event.h"				            /* Window event handling functions */
#include "camera.h"                             /* Camera handling function */
#include "render.h"								/* Render functions */

#define SCREEN_WIDTH 800			/* Screen width */
#define SCREEN_HEIGHT 600			/* Screen height */

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
    // u32 chunkId;    /* Chunk ID */
}   t_block;

struct compact_cube {
	u32 shape_texture;
	/*
		u16 shape;
		u16 texture;
	*/
	u32 y_light;
	/*
		u8 flag;
		u8 torch_sun_light;  4 bits for torch light and 4 bits for sun light
		u16 y;
	*/
	u32 x;		/* x position */
	u32 z;		/* z position */
};


#define WORLD_MAX_HEIGHT 256
#define WORLD_MAX_WIDTH (16384 * 2)
#define WORLD_MAX_DEPTH (16384 * 2)

#define CHUNKS_HEIGHT   16U
#define CHUNKS_WIDTH    16U
#define CHUNKS_DEPTH    16U

#define BLOCK_PER_CHUNKS (CHUNKS_HEIGHT * CHUNKS_WIDTH * CHUNKS_DEPTH) 

#define VERTEX_SHADER_PATH		"rsc/shaders/vertex_shader.glsl"
#define FRAGMENT_SHADER_PATH	"rsc/shaders/fragment_shader.glsl"

typedef struct s_chunks {
    t_block blocks[CHUNKS_WIDTH][CHUNKS_HEIGHT][CHUNKS_DEPTH];   /* Blocks array */
    u32     id;     		/* Chunk Id */
	u32		nb_block;		/* nb block to give to render context */
} t_chunks;

/* Context structure */
typedef struct s_context {
	t_camera	cam;			/* camera structure */
    GLFWwindow	*win_ptr;		/* Window pointer */
	t_modelCube	cube;			/* Data Cube structure */
    t_chunks    *chunks;         /* current chunk */
	GLuint		shader_id;		/* shader program id */
} t_context;

u32 chunks_cube_get(t_chunks *chunks, vec3_f32 *block_array);

/* render/cube.c */
GLuint setupCubeVAO(t_context *c, t_modelCube *cube);
void	drawCube(GLuint vao, u32 nb_cube);


#endif /* VOX_HEADER_H */