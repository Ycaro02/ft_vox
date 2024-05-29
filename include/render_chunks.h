#ifndef HEADER_RENDER_CHUNKS_H
#define HEADER_RENDER_CHUNKS_H

#include "chunks.h"

typedef struct s_render_chunks {
    // vec3		*block_array;		/* Block array (VBO data represent block instance position) */
    // GLuint		instanceVBO;		/* Instance VBO */
	// GLuint  	typeBlockVBO;		/* Type VBO */
	// f32			*blockTypeID;		/* Block type ID */
    // u32			visibleBlock;		/* Number of visible block in this chunks */
	BlockPos 	chunkID;			/* Chunk ID, (0, offsetX, offsetZ) */


	vec3		*faceArray[6];
	f32			*faceTypeID[6];
	GLuint		faceVBO[6];
	GLuint		faceTypeVBO[6];
	u32			*faceCount;
} RenderChunks;

/* render/render_chunks.c */
void			renderChunkFree(RenderChunks *render);
/* Function to provide at hashmap init for renderChunks map */
void			renderChunksMapFree(void *entry);

/* Render chunks.c */
void			chunksCubeGet(Chunks *chunks, RenderChunks *render);
RenderChunks 	*renderChunkCreate(Context *c, Chunks *chunks);
RenderChunks	*renderChunkCreateVBO(Mutex *chunkMtx, HashMap *chunksMap, BlockPos chunkID);

/* render/load_chunks.c */
s8				chunksIsRenderer(HashMap *renderChunksMap, BlockPos chunkID);
s8				chunkIsLoaded(HashMap *chunksMap, BlockPos chunkID);
void			chunksViewHandling(Context *c);
void 			renderChunksFrustrumRemove(Context *c, HashMap *renderChunksMap);

/* render/occlusion_culling */
u32				checkHiddenBlock(Chunks *chunks, u32 subChunksID);
void			updateNeighbors(Block *block, Block *blockCache[16][16][16]);

void updateTopBotNeighbors(SubChunks *botSubChunk, Block *topBlockCache[16][16][16]);

/* render/cube.c to be refact */
void			drawAllCube(GLuint VAO, RenderChunks *render);

/* render/loadchunks.c */
s8 				chunkIsLoaded(HashMap *chunksMap, BlockPos chunkID);
s8 				chunksRenderIsLoaded(Chunks *chunk);
s8 				chunksIsRenderer(HashMap *renderChunksMap, BlockPos chunkID);

void renderChunksVBODestroy(Context *c);


/* render/cube_face_build.c */
s8				faceVisible(u8 neighbors, u8 face);
void			chunksCubeFaceGet(Chunks *chunks, RenderChunks *render);
GLuint			faceInstanceVBOCreate(vec3 *faceArray, u32 faceNb);
RenderChunks	*renderChunkCreateFaceVBO(Mutex *chunkMtx, HashMap *chunksMap, BlockPos chunkID);
void			drawAllChunksByFace(Context *c);

#endif /* HEADER_RENDER_CHUNKS_H */
