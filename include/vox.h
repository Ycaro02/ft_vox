#ifndef VOX_HEADER_H
#define VOX_HEADER_H

#include <math.h>					            /* LIBC Math functions */
#include "glad/gl.h"				            /* Glad functions (Include glad header BEFORE glfw3) */
#include "glfw3/glfw3.h"			            /* GLFW functions */

#include "../rsc/deps/cglm/include/cglm/cglm.h" 

#include "../libft/libft.h"		                /* Libft functions */
#include "../libft/parse_flag/parse_flag.h"		/* Parse flag functions */
#include "../libft/HashMap/HashMap.h"			/* Hashmap functions */
#include "../libft/BMP_parser/parse_bmp.h"		/* BMP parser functions */
#include "window.h"					            /* Window related functions */
#include "win_event.h"				            /* Window event handling functions */
#include "camera.h"                             /* Camera handling function */
#include "render.h"								/* Render functions */
#include "chunks.h"								/* Chunks functions */


/* Screen size */
#define SCREEN_WIDTH	1366			/* Screen width */
#define SCREEN_HEIGHT	768			/* Screen height */

/* Texture atlas path */
#define TEXTURE_ATLAS_PATH "rsc/texture/texture_atlas.bmp"

/* Sky path */
#define TEXTURE_SKY_PATH "rsc/texture/skybox.bmp"

/* Block hiden deine */
#define BLOCK_HIDDEN 1U

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

enum BlockType {
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
}   Block;


typedef struct s_world {
	u64			seed;			/* World seed */
	HashMap		*chunksMap;		/* Chunks hashmap */
	u32			chunksLoaded;	/* Number of chunks loaded */
} World;

/* Context structure */
typedef struct s_context {
	World		*world;				/* World structure */
	Camera	cam;				/* camera structure */
    GLFWwindow	*win_ptr;			/* Window pointer */
	ModelCube	cube;				/* Data Cube structure */
    // Chunks    *chunks;        	/* current chunk */
	GLuint		cubeShaderID;		/* shader program id */
	GLuint		skyboxShaderID;		/* shader program id */
	GLuint		skyboxVAO;				/* skybox VAO */
	u32			renderBlock;		/* Total block to render */
} Context;

/* Atlas texture ID */
enum AtlasId {
	ATLAS_DIRT1=0, /* Pink to remove */
	ATLAS_SAND=1,
	ATLAS_STONE_CUT=2,
	ATLAS_BRICK=3,
	ATLAS_WOOD=4,
	ATLAS_STONE=5,
	ATLAS_DIRT2=6,
	ATLAS_WOOD_PLANK=7, /* same here */
	ATLAS_DIRT3=8,
	ATLAS_GLASS=9,
	ATLAS_COBBLESTONE=10,
	ATLAS_FULL_GREY=11,
	ATLAS_STONE_CLEAN=12,
	ATLAS_CRAFTING_TABLE=13,
};


/* render/cube.c */
GLuint	setupCubeVAO(Context *c, ModelCube *cube);
void	drawAllCube(GLuint vao, GLuint vbo, u32 nb_cube);

/* texture load_texture */
GLuint *load_texture_atlas(char *path, int squareHeight, int squareWidth, vec3_u8 ignore_color);
void set_shader_texture(GLuint shaderId, GLuint *atlas, u32 index, u32 textureType);	
GLuint load_cubemap(char* path, int squareHeight, int squareWidth, vec3_u8 ignore_color);
/* render/occlusion_culling */
u32 checkHiddenBlock(Chunks *chunks, u32 subChunksID);


/* cube.c */
GLuint bufferGlCreate(GLenum type, u32 size, void *data);


/* chunks.c */
t_list *chunksToRenderChunks(Context *c, HashMap *chunksMap);
void renderChunkFree(RenderChunks *render);

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
