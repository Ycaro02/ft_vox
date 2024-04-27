#ifndef VOX_HEADER_H
#define VOX_HEADER_H

#include <math.h>					            /* LIBC Math functions */
#include "glad/gl.h"				            /* Glad functions (Include glad header BEFORE glfw3) */
#include "glfw3/glfw3.h"			            /* GLFW functions */
#include "../libft/libft.h"		                /* Libft functions */
#include "../libft/parse_flag/parse_flag.h"    /* Parse flag functions */
#include "window.h"					            /* Window related functions */
#include "win_event.h"				            /* Window event handling functions */

#define SCREEN_WIDTH 800			/* Screen width */
#define SCREEN_HEIGHT 600			/* Screen height */

enum block_type {
    AIR=0U,      /* Air block (Empty) */
    DIRT=1U,     /* Dirt block (Exposed to light) */
    STONE=2U,    /* Stone block (Underground) */
};

typedef struct s_block {
    s32 x;          /* Block x position */
    s32 y;          /* Block y position (height) */
    s32 z;          /* Block z position */
    u32 type;       /* Block type */
    u32 chunkId;    /* Chunk ID */
}   t_block;

#define WORLD_MAX_HEIGHT 256
#define WORLD_MAX_WIDTH (16384 * 2)
#define WORLD_MAX_DEPTH (16384 * 2)

#define CHUNKS_HEIGHT   32
#define CHUNKS_WIDTH    32
#define CHUNKS_DEPTH    32

typedef struct s_chunks {
    t_block blocks[CHUNKS_HEIGHT][CHUNKS_WIDTH][CHUNKS_DEPTH];   /* Blocks array */
    u32 id; /* Chunk Id */
} t_chunks;

/* Context structure */
typedef struct s_context {
    GLFWwindow *win_ptr;			/* Window pointer */
} t_context;



#endif /* VOX_HEADER_H */