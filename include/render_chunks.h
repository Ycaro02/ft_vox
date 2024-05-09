#ifndef HEADER_RENDER_CHUNKS_H
#define HEADER_RENDER_CHUNKS_H

#include "vox.h"

typedef struct s_render_chunks {
    vec3	*block_array;		/* Block array (VBO data represent block instance position) */
    GLuint	instanceVBO;		/* Instance VBO */
	GLuint  typeBlockVBO;		/* Type VBO */
	f32		*blockTypeID;		/* Block type ID */
    u32		visibleBlock;		/* Number of visible block in this chunks */
} RenderChunks;

/* render/render_chunks.c */
t_list *chunksToRenderChunks(Context *c, HashMap *chunksMap);
void renderChunkFree(RenderChunks *render);

/* Render chunks.c */
u32		chunksCubeGet(Chunks *chunks, RenderChunks *render, u32 chunkID);
void	chunksLoadArround(Context *c, s32 radius);

/* render/cube.c*/
void drawAllCube(GLuint VAO, RenderChunks *render);

#endif /* HEADER_RENDER_CHUNKS_H */
