#ifndef HEADER_RENDER_CHUNKS_H
#define HEADER_RENDER_CHUNKS_H

#include "chunks.h"

typedef struct s_render_chunks {
	vec3		*faceArray[6];
	f32			*faceTypeID[6];
	GLuint		faceVBO[6];
	GLuint		faceTypeVBO[6];
	u32			*faceCount;
	BlockPos 	chunkID;			/* Chunk ID, (0, offsetX, offsetZ) */
	suseconds_t lastUpdate;			/* Last update time */
} RenderChunks;

/* render/render_chunks.c */
void			renderChunkFree(RenderChunks *render);
/* Function to provide at hashmap init for renderChunks map */
// void			renderChunksMapFree(void *entry);

void addRenderToVBOCreate(Context *c, BlockPos chunkID);

/* Render chunks.c */
void			chunksCubeGet(Chunks *chunks, RenderChunks *render);
RenderChunks 	*renderChunkCreate(Context *c, Chunks *chunks);
RenderChunks	*renderChunkCreateVBO(Mutex *chunkMtx, HashMap *chunksMap, BlockPos chunkID);

/* render/load_chunks.c */
s8				chunksIsRenderer(HashMap *renderChunksMap, BlockPos chunkID);
s8				chunkIsLoaded(HashMap *chunksMap, BlockPos chunkID);
void			chunksViewHandling(Context *c);
void 			renderChunksFrustrumRemove(Context *c, HashMap *renderChunksMap);
void 			renderChunksVBODestroyListBuild(Context *c, Chunks *chunk);

/* render/occlusion_culling */
// u32				checkHiddenBlock(Chunks *chunks, u32 subChunksID);
void			updateNeighbors(Block *block, Block ****blockCache);
void			updateTopBotNeighbors(SubChunks *botSubChunk, Block ****topBlockCache);
void			chunkNeighborsGet(Context *c, Chunks *chunk, Chunks *neighborChunksCache[4]);
void			updateChunkNeighbors(Context *c, Chunks *chunk, Block *****chunkBlockCache, Chunks *neighborChunksCache[4]);
void			chunkNeighborMaskUpdate(Context *c, Chunks *chunk);


/* render/loadchunks.c */
s8 				chunkIsLoaded(HashMap *chunksMap, BlockPos chunkID);
s8 				chunksRenderIsLoaded(Chunks *chunk);
s8 				chunksIsRenderer(HashMap *renderChunksMap, BlockPos chunkID);


/* render/cube_face_build.c */
s8				faceVisible(u8 neighbors, u8 face);
void			chunksCubeFaceGet(Mutex *chunkMtx, Chunks *chunks, RenderChunks *render);
GLuint			faceInstanceVBOCreate(vec3 *faceArray, u32 faceNb);
void			renderChunkCreateFaceVBO(HashMap *chunksMap, BlockPos chunkID);
void			drawAllChunksByFace(Context *c);

#endif /* HEADER_RENDER_CHUNKS_H */
