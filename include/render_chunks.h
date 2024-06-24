#ifndef HEADER_RENDER_CHUNKS_H
#define HEADER_RENDER_CHUNKS_H

#include "typedef_struct.h"

struct s_render_chunks {
	vec3		*faceArray[6];
	s32			*faceTypeID[6];
	GLuint		faceVBO[6];
	GLuint		faceTypeVBO[6];
	u32			*faceCount;

	vec3		*trspFaceArray[6];
	s32			*trspTypeId[6];
	GLuint		trspFaceVBO[6];
	GLuint		trspTypeVBO[6];
	u32 		*trspFaceCount;

	BlockPos 	chunkID;			/* Chunk ID, (0, offsetX, offsetZ) */
	suseconds_t lastUpdate;			/* Last update time */
};

struct s_underground_block {
	u8		isUnderground;
	vec3	*udgFaceArray[6];
	s32		*udgTypeID[6];
	u32 	udgFaceCount;
	GLuint	udgFaceVBO[6];
	GLuint	udgTypeVBO[6];
};

/* render/render_chunks.c */
void			renderChunkFree(RenderChunks *render);
void			addRenderToVBOCreate(Context *c, BlockPos chunkID);

/* Render chunks.c */
RenderChunks 	*renderChunkCreate(Context *c, Chunks *chunks);
RenderChunks	*renderChunkCreateVBO(Mutex *chunkMtx, HashMap *chunksMap, BlockPos chunkID);

/* render/load_chunks.c */
s8				chunksIsRenderer(HashMap *renderChunksMap, BlockPos chunkID);
s8				chunkIsLoaded(HashMap *chunksMap, BlockPos chunkID);
void			chunksViewHandling(Context *c);
void 			renderChunksFrustrumRemove(Context *c, HashMap *renderChunksMap);
void 			renderChunksVBODestroyListBuild(Context *c, Chunks *chunk);

/* render/occlusion_culling */
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
void			chunksCubeFaceGet(Mutex *chunkMtx, Chunks *chunks, RenderChunks *render);
GLuint			faceInstanceVBOCreate(vec3 *faceArray, u32 faceNb);
void			renderChunkCreateFaceVBO(HashMap *chunksMap, BlockPos chunkID);
void			drawAllChunksByFace(Context *c);
s8				isTransparentBlock(s32 type);
#endif /* HEADER_RENDER_CHUNKS_H */
