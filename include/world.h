#ifndef HEADER_WORLD_H
#define HEADER_WORLD_H

#include "vox.h"
#include "cube.h"


typedef struct s_camera Camera;
typedef struct s_thread_entity ThreadEntity;
typedef mtx_t Mutex;
typedef thrd_t Thread;

typedef struct s_world {
	u64				seed;			/* World seed */
	HashMap			*chunksMap;		/* Chunks hashmap */
} World;

typedef struct s_thread_context {
	Thread			supervisor;		/* Thread supervisor */
	Mutex 			mtx;			/* Mutex to protect data */
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
	GLuint				cubeShaderID;		/* shader program id */
	GLuint				skyboxShaderID;		/* shader program id */
	GLuint				skyboxVAO;			/* skybox VAO */
	u32					renderBlock;		/* Total block to render */
	u8					*perlinNoise;		/* perlinNoise data */
	ThreadContext		*threadContext;		/* Thread context */
	// Thread				threadSupervisor;	/* Thread supervisor */
	// WorkerThread		*thread;			/* Thread structure array */
	// Mutex				mtx;				/* Mutex */
} Context;

/* RenderChunks ID in renderChunksHashmap, same id than CHUNKS_MAP_ID_GET(Chunks) */
#define RENDER_CHUNKS_ID(r) ((BlockPos)r->chunkID)

/* Chunks ID in chunksHashmap, same id than RENDER_CHUNKS_ID(RenderChunks) */
#define CHUNKS_MAP_ID_GET(offsetX, offsetZ) ((BlockPos){0, offsetX, offsetZ})


/* cube.c */
GLuint bufferGlCreate(GLenum type, u32 size, void *data);


void display_camera_value(Context *context);

#endif /* HEADER_WORLD_H */
