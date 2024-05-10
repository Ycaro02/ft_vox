#ifndef HEADER_WORLD_H
#define HEADER_WORLD_H

#include "vox.h"

typedef struct s_world {
	u64			seed;			/* World seed */
	HashMap		*chunksMap;		/* Chunks hashmap */
	u32			chunksLoaded;	/* Number of chunks loaded */
} World;

/* Context structure */
typedef struct s_context {
	World		*world;				/* World structure */
	Camera		cam;				/* camera structure */
    GLFWwindow	*win_ptr;			/* Window pointer */
	ModelCube	cube;				/* Data Cube structure */
    // Chunks    *chunks;        	/* current chunk */
	GLuint		cubeShaderID;		/* shader program id */
	GLuint		skyboxShaderID;		/* shader program id */
	GLuint		skyboxVAO;				/* skybox VAO */
	u32			renderBlock;		/* Total block to render */
	u8			*perlinNoise;		/* perlinNoise data */
} Context;

/* render/cube.c */
GLuint	setupCubeVAO(Context *c, ModelCube *cube);


/* cube.c */
GLuint bufferGlCreate(GLenum type, u32 size, void *data);


#endif /* HEADER_WORLD_H */
