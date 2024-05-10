#ifndef HEADER_RENDER_CHUNKS_H
#define HEADER_RENDER_CHUNKS_H

#include "chunks.h"

typedef struct s_render_chunks {
    vec3		*block_array;		/* Block array (VBO data represent block instance position) */
    GLuint		instanceVBO;		/* Instance VBO */
	GLuint  	typeBlockVBO;		/* Type VBO */
	f32			*blockTypeID;		/* Block type ID */
    u32			visibleBlock;		/* Number of visible block in this chunks */
	BlockPos 	chunkID;			/* Chunk ID, (0, offsetX, offsetZ) */
} RenderChunks;

/* render/render_chunks.c */
HashMap			*chunksToRenderChunks(Context *c, HashMap *chunksMap);
void			renderChunkFree(RenderChunks *render);

/* Render chunks.c */
u32				chunksCubeGet(Chunks *chunks, RenderChunks *render);
void			chunksLoadArround(Context *c, s32 radius);
RenderChunks 	*renderChunkCreate(Chunks *chunks);

/* render/load_chunks.c */
s8				chunksIsRenderer(HashMap *renderChunksMap, BlockPos chunkID);
s8				chunkIsLoaded(HashMap *chunksMap, BlockPos chunkID);
void			chunksViewHandling(Context *c, HashMap *renderChunksMap);


/* render/occlusion_culling */
u32 checkHiddenBlock(Chunks *chunks, u32 subChunksID);

/* render/cube.c to be refact */
void	drawAllCube(GLuint VAO, RenderChunks *render);


#endif /* HEADER_RENDER_CHUNKS_H */
