#ifndef HEADER_WORLD_H
#define HEADER_WORLD_H

#include "typedef_struct.h"

struct s_noise_generation {
	f32					**continental;		/* Continental noise 2D */
	f32					**erosion;			/* Erosion noise */
	f32					**peaksValley;		/* Peaks and valley noise */
	f32					**humidity;			/* Humidity noise */
	f32					**temperature;		/* Temperature noise */
	u8					**cave;				/* 2D noise for cave */
} ;

struct s_world {
	u64					seed;					/* World seed */
	HashMap				*chunksMap;				/* Chunks hashmap */
	HashMap				*renderChunksMap;		/* Render chunks map */
	UndergroundBlock 	*undergroundBlock;		/* Underground block */
	NoiseGeneration		noise;
};

struct s_thread_context {
	Thread			supervisor;			/* Thread supervisor */
	Mutex 			chunkMtx;			/* Mutex to protect data, used for chunks hashmap  */
	Mutex 			threadMtx;			/* Mutex to protect thread, used for thread status and chunk queue loading */
	Mutex			logMtx;
	HashMap 		*chunksMapToLoad;	/* Chunks queue to load, (for now contain ThreadData struct) */
    ThreadEntity	*workers;			/* Worker thread array */
	s64         	workerMax;			/* Maximum of worker thread (size of workers array) */
};

typedef struct s_noise_data {
	f32				valContinental;		/* Continental value */
	f32				valErosion;			/* Erosion value */
	f32				valPeaksValley;		/* Peaks and valley value */
	f32				valCombined;		/* Combined value */
	f32				valHumidity;		/* Humidity value */
	f32				valTemperature;		/* Temperature value */
} NoiseData;

typedef struct s_display_data {
	u32				chunkRenderedNb;	/* Chunk rendered */
	u32				chunkLoadedNb;		/* Chunk loaded */
	u32				chunkToLoadInQueue;	/* Chunk to load in queue */
	u32				faceRendered;		/* Face rendered */
	s32				chunkX;				/* Chunk X */
	s32				chunkZ;				/* Chunk Z */
	BlockPos		blockPos;			/* Block position */
	NoiseData		noiseData;			/* Noise data */
	// vec2_s32		posNoise;			/* Position */
} DisplayData;

/* Context structure */
struct s_context {
	Camera				*cam;				/* camera structure */
	World				*world;				/* World structure */
    GLFWwindow			*win_ptr;			/* Window pointer */
	FaceCubeModel		*faceCube;			/* Data Face Cube structure */
	ThreadContext		*threadContext;		/* Thread context */
	/* Font handling can be in is own struct */
	FontContext			*fontContext;		/* Font context */
	/* Vbo handling list can be with mutex  */
	t_list				*vboToDestroy;		/* VBO to destroy */
	t_list				*vboToCreate;		/* VBO to create */
	/* Mutex protect can be in is own struct */
	Mutex				renderMtx;				/* Mutex to protect VBO, used for renderChunks map */
	Mutex				gameMtx;				/* Mutex to protect game, used for game boolean and cam chunk Pos */
	Mutex				isRunningMtx;			/* Mutex to protect isRunning */
	Mutex				vboToDestroyMtx;		/* Mutex to protect vboToDestroy list */
	Mutex				vboToCreateMtx;			/* Mutex to protect vboToCreate list */
	Mutex				renderDataNeededMtx;	/* Mutex to protect renderDataNeeded */
	/* Opengl ID for shader vao or texture (openGl context) */
	s32					screenHeight;		/* Screen height */
	s32					screenWidth;		/* Screen width */
	GLuint				cubeShaderID;		/* shader program ID */
	GLuint				skyboxShaderID;		/* shader program ID */
	GLuint				skyboxVAO;			/* skybox VAO */
	GLuint				blockAtlasId;		/* block texture ID */
	GLuint				skyTexture;			/* skybox texture ID */
	mat4				rotation;			/* rotation matrix */
	/* Data displayed */
	DisplayData			displayData;		/* Display data */
	/* Multiple bool can be in the same var and handle with power of 2 (flag )*/
	s8					isPlaying;			/* Game is playing */
	u8					renderDataNeeded; 	/* Render need data, bool to specify render want to lock data (chunkMtx) */
	s8					autoMove;			/* Auto move camera */
	s8					autoRotate;			/* Auto rotate camera */
	s8					displayUndergroundBlock; /* Display underground block */
};


/* RenderChunks ID in renderChunksHashmap, same id than CHUNKS_MAP_ID_GET(Chunks) */
#define RENDER_CHUNKS_ID(r) ((BlockPos)r->chunkID)

/* Chunks ID in chunksHashmap, same id than RENDER_CHUNKS_ID(RenderChunks) */
#define CHUNKS_MAP_ID_GET(offsetX, offsetZ) ((BlockPos){0, offsetX, offsetZ})

#define VOX_PROTECTED_LOG(c, msg, ...) \
	do { \
		mtx_lock(&(c->threadContext->logMtx)); \
		ft_printf_fd(1, msg, ##__VA_ARGS__); \
		mtx_unlock(&(c->threadContext->logMtx)); \
	} while (0)


FT_INLINE void renderNeedDataSet(Context *c, u8 value) {
	mtx_lock(&c->renderDataNeededMtx);
	c->renderDataNeeded = value;
	mtx_unlock(&c->renderDataNeededMtx);
}

FT_INLINE s8 renderNeedDataGet(Context *c) {
	s8 value = FALSE;

	mtx_lock(&c->renderDataNeededMtx);
	value = c->renderDataNeeded;
	mtx_unlock(&c->renderDataNeededMtx);
	return (value);
}


FT_INLINE s8 voxIsRunning(Context *context) {
	s8 playing = TRUE;
	mtx_lock(&context->isRunningMtx);
	playing = context->isPlaying;
	mtx_unlock(&context->isRunningMtx);
	return (playing);
}


// FT_INLINE void computeTimeSpend(TimeSpec *start, TimeSpec *end, f64 *time) {
// 	*time = (f64)(end->tv_sec - start->tv_sec) + (f64)(end->tv_nsec - start->tv_nsec) / 1e9;
// }

// FT_INLINE void mtxLockUpdateTime(Context *c, Mutex *mtx, TimeSpec *start, TimeSpec *end, f64 *time, char *mtxName) {
// 	mtx_lock(mtx);
// 	clock_gettime(CLOCK_MONOTONIC, start);
// 	clock_gettime(CLOCK_MONOTONIC, end);
// 	computeTimeSpend(start, end, time);
// 	VOX_PROTECTED_LOG(c, "%s mtx time: %f\n", mtxName, *time);
// }


/* cube.c */
GLuint bufferGlCreate(GLenum type, u32 size, void *data);


void display_camera_value(Context *context);


Context *contextInit();
u8 *perlinNoiseGeneration(unsigned int seed);

// s8 contextTextureInit(Context *context);
#endif /* HEADER_WORLD_H */
