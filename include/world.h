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
	Thread			supervisor;			/* Thread supervisor */
	Mutex 			chunkMtx;			/* Mutex to protect data, used for chunks hashmap  */
	Mutex 			threadMtx;			/* Mutex to protect thread, used for thread status and chunk queue loading */
	Mutex			logMtx;
	HashMap 		*chunksMapToLoad;	/* Chunks queue to load, (for now contain ThreadData struct) */
    ThreadEntity	*workers;			/* Worker thread array */
	s64         	workerMax;			/* Maximum of worker thread (size of workers array) */
} ThreadContext;

typedef struct timespec TimeSpec;

typedef struct s_mutex_time {
	TimeSpec	start;
	TimeSpec	end;
	f64			gameMtxTime;
	f64			renderMtxTime;
	f64			isRunningMtxTime;
	f64			vboToDestroyMtxTime;
	f64			vboToCreateMtxTime;
	f64			chunkMtxTime;
	f64			threadMtxTime;
	f64			logMtxTime;
} MutexTime;

/* Context structure */
typedef struct s_context {
	World				*world;				/* World structure */
	Camera				cam;				/* camera structure */
    GLFWwindow			*win_ptr;			/* Window pointer */
	// ModelCube			cube;				/* Data Cube structure */
	FaceCubeModel		*faceCube;			/* Data Face Cube structure */
	u32					renderBlock;		/* Total block to render */
	ThreadContext		*threadContext;		/* Thread context */
	s8					isPlaying;			/* Game is playing */
	f32					**perlin2D;			/* Perlin noise 2D */
	f32					**perlinCaveNoise;		/* Perlin noise 2D for cave */
	t_list				*vboToDestroy;		/* VBO to destroy */
	t_list				*vboToCreate;		/* VBO to create */
	Mutex				renderMtx;				/* Mutex to protect VBO, used for renderChunks map */
	Mutex				gameMtx;				/* Mutex to protect game, used for game boolean and cam chunk Pos */
	Mutex				isRunningMtx;		/* Mutex to protect isRunning */
	Mutex				vboToDestroyMtx;	/* Mutex to protect vboToDestroy list */
	Mutex				vboToCreateMtx;		/* Mutex to protect vboToCreate list */
	GLuint				cubeShaderID;		/* shader program id */
	GLuint				skyboxShaderID;		/* shader program id */
	GLuint				skyboxVAO;			/* skybox VAO */
	// GLuint				cubeVAO;			/* cube VAO */
	GLuint				skyTexture;			/* skybox VAO */
	mat4				rotation;		/* rotation matrix */
	MutexTime			mtxTime;		/* Mutex time */
} Context;


#define VOX_PROTECTED_LOG(c, msg, ...) \
    do { \
        mtx_lock(&(c->threadContext->logMtx)); \
        ft_printf_fd(1, msg, ##__VA_ARGS__); \
        mtx_unlock(&(c->threadContext->logMtx)); \
    } while (0)

FT_INLINE void computeTimeSpend(TimeSpec *start, TimeSpec *end, f64 *time) {
	*time = (f64)(end->tv_sec - start->tv_sec) + (f64)(end->tv_nsec - start->tv_nsec) / 1e9;
}

FT_INLINE void mtxLockUpdateTime(Context *c, Mutex *mtx, TimeSpec *start, TimeSpec *end, f64 *time, char *mtxName) {
	mtx_lock(mtx);
	clock_gettime(CLOCK_MONOTONIC, start);
	clock_gettime(CLOCK_MONOTONIC, end);
	computeTimeSpend(start, end, time);
	VOX_PROTECTED_LOG(c, "%s mtx time: %f\n", mtxName, *time);
}


/* RenderChunks ID in renderChunksHashmap, same id than CHUNKS_MAP_ID_GET(Chunks) */
#define RENDER_CHUNKS_ID(r) ((BlockPos)r->chunkID)

/* Chunks ID in chunksHashmap, same id than RENDER_CHUNKS_ID(RenderChunks) */
#define CHUNKS_MAP_ID_GET(offsetX, offsetZ) ((BlockPos){0, offsetX, offsetZ})


/* cube.c */
GLuint bufferGlCreate(GLenum type, u32 size, void *data);


void display_camera_value(Context *context);


FT_INLINE s8 voxIsRunning(Context *context) {
	s8 playing = TRUE;
	mtx_lock(&context->isRunningMtx);
	playing = context->isPlaying;
	mtx_unlock(&context->isRunningMtx);
	return (playing);
}

Context *contextInit();
u8 *perlinNoiseGeneration(unsigned int seed);

// s8 contextTextureInit(Context *context);
#endif /* HEADER_WORLD_H */
