#ifndef HEADER_TYPEDEF_STRUCT_H
#define HEADER_TYPEDEF_STRUCT_H

#include "../rsc/deps/cglm/include/cglm/cglm.h" /* CGLM C maths openGL */
#include "glad/gl.h"				            /* Glad functions (Include glad header BEFORE glfw3) */
#include "glfw3/glfw3.h"			            /* GLFW functions */
#include <math.h>					            /* LIBC Math functions */
#include "../libft/tinycthread.h"        /* TinyCThread functions */
#include "../libft/libft.h"							/* Libft functions */
#include "../libft/HashMap/HashMap.h"				/* Hashmap functions */
#include "../libft/parse_flag/parse_flag.h"		/* Parse flag functions */
#include "../libft/BMP_parser/parse_bmp.h"		/* BMP parser functions */

/* Tinycthread */
typedef 		mtx_t					Mutex;
typedef 		thrd_t					Thread;

/* World.h */
typedef struct	s_context				Context;
typedef struct	s_thread_context		ThreadContext;
typedef struct 	s_world 				World;

/* thread_load.h */
typedef struct 	s_thread_entity 		ThreadEntity;
typedef struct	s_thread_data 			ThreadData;

/* chunks.h */
typedef struct	s_chunks				Chunks;
typedef struct	s_sub_chunks 			SubChunks;

/* render_chunks.h*/
typedef struct	s_render_chunks			RenderChunks;

/* perlin_nosie.h */
typedef struct	s_perlin_data 			PerlinData;

/* block.h */
typedef struct	s_block 				Block;
typedef struct	s_underground_block		UndergroundBlock;

/* camera.h */
typedef struct	s_camera 				Camera;
typedef struct	s_bounding_box 			BoundingBox;
typedef struct	s_frustrum 				Frustum;


/* win_event.h */
typedef struct	s_key_action 			KeyAction;

/* cube.h */
typedef struct	s_vertex_texture		VertexTexture;
typedef struct	s_modelCube				ModelCube;
typedef struct	s_faceCubeModel			FaceCubeModel;

/* Character font */
typedef struct	s_font_context			FontContext;
typedef struct	s_character_font 		CharacterFont;


#endif /* HEADER_TYPEDEF_STRUCT_H */
