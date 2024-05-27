#ifndef HEADER_WORLD_H
#define HEADER_WORLD_H

#include "vox.h"
#include "cube.h"

typedef struct s_camera Camera;
typedef struct s_thread_entity ThreadEntity;
typedef mtx_t Mutex;
typedef thrd_t Thread;

typedef struct s_world {
	u64				seed;					/* World seed */
	HashMap			*chunksMap;				/* Chunks hashmap */
	HashMap			*renderChunksMap;		/* Render chunks map */
	// HashMap			*renderChunksCacheMap;	/* Cache of renderchunks to avoid multiple creation/destroy of VBO */
} World;

typedef struct s_thread_context {
	Thread			supervisor;		/* Thread supervisor */
	Mutex 			chunkMtx;			/* Mutex to protect data, used for chunks hashmap  */
	Mutex 			threadMtx;			/* Mutex to protect thread, used for thread status and chunk queue loading */
	HashMap 		*chunksMapToLoad;	/* Chunks queue to load, (for now contain ThreadData struct) */
    ThreadEntity	*workers;		/* Worker thread array */
	s64         	workerMax;		/* Maximum of worker thread (size of workers array) */
    s64         	workerCurrent;	/* Current busy worker thread */
} ThreadContext;

/* Context structure */
typedef struct s_context {
	World				*world;				/* World structure */
	Camera				cam;				/* camera structure */
    GLFWwindow			*win_ptr;			/* Window pointer */
	ModelCube			cube;				/* Data Cube structure */
	u32					renderBlock;		/* Total block to render */
	ThreadContext		*threadContext;		/* Thread context */
	s8					isPlaying;			/* Game is playing */
	f32					**perlin2D;			/* Perlin noise 2D */
	t_list				*vboToDestroy;		/* VBO to destroy */
	// t_list				*vboToCreate;		/* VBO to create */
	Mutex				renderMtx;				/* Mutex to protect VBO, used for renderChunks map */
	Mutex				gameMtx;				/* Mutex to protect game, used for game boolean and cam chunk Pos */
	GLuint				cubeShaderID;		/* shader program id */
	GLuint				skyboxShaderID;		/* shader program id */
	GLuint				skyboxVAO;			/* skybox VAO */
	GLuint				cubeVAO;			/* cube VAO */
	GLuint				skyTexture;			/* skybox VAO */
} Context;

/* RenderChunks ID in renderChunksHashmap, same id than CHUNKS_MAP_ID_GET(Chunks) */
#define RENDER_CHUNKS_ID(r) ((BlockPos)r->chunkID)

/* Chunks ID in chunksHashmap, same id than RENDER_CHUNKS_ID(RenderChunks) */
#define CHUNKS_MAP_ID_GET(offsetX, offsetZ) ((BlockPos){0, offsetX, offsetZ})


/* cube.c */
GLuint bufferGlCreate(GLenum type, u32 size, void *data);


void display_camera_value(Context *context);


FT_INLINE s8 voxIsRunning(Context *context) {
	s8 playing = TRUE;
	mtx_lock(&context->gameMtx);
	playing = context->isPlaying;
	mtx_unlock(&context->gameMtx);
	return (playing);
}

Context *contextInit();
u8 *perlinNoiseGeneration(unsigned int seed);
f32 **array1DTo2D(u8 *array, u32 height, u32 width);
f32 normalizeU8Tof32(u8 value, u8 start1, u8 stop1, f32 start2, f32 stop2);


s8 contextTextureInit(Context *context);

#endif /* HEADER_WORLD_H */
