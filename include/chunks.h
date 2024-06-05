#ifndef HEADER_CHUNKS_H
#define HEADER_CHUNKS_H

#include "world.h"

#define CHUNKS_HEIGHT   	256U	/* that will be 256 */
#define SUB_CHUNKS_HEIGHT   16U

/* Number of subchunks in chunk  */
#define SUB_CHUNKS_MAX (CHUNKS_HEIGHT / SUB_CHUNKS_HEIGHT)

typedef struct s_render_chunks RenderChunks;

typedef struct s_sub_chunks {
	HashMap 		*block_map;		/* Blocks map, use HashMap API to set/get block */
} SubChunks;

typedef struct s_perlin_data PerlinData;

typedef struct s_chunks {
	SubChunks		sub_chunks[SUB_CHUNKS_MAX]; /* array of sub_chunks */
	u32				nb_block;					/* nb block (outdated value total of blockmap subchunk) */
	// u32				visible_block;				/* Number of visible block */
	s32				x;							/* Chunk offset X can be negative  (relative to origine chunks) */	
	s32				z;							/* Chunk offset Z can be negative (relative to origine chunks) */
	suseconds_t		lastUpdate;					/* Last update time */
	PerlinData		**perlinVal;				/* Perlin noise value */
	PerlinData		**perlinCave;				/* Perlin noise cave value */
	RenderChunks 	*render;					/* RenderChunks pointer */
	u8				neighbors;					/* Neighbors */
} Chunks;


void 	chunksMapFree(void *entry);
Chunks *getChunkAt(Context *c, s32 x, s32 z);
u32 	subChunksMaxGet(Chunks *chunk);
/* chunks */
Chunks *chunksLoad(Block *chunkBlockCache[16][16][16][16], Mutex *mtx, f32 **perlin2D, s32 chunkX, s32 chunkZ, f32 **perlinCavenoise);
s32 chunksEuclideanDistanceGet(s32 camChunkX, s32 camChunkZ, s32 chunkX, s32 chunkZ);
void unloadChunkHandler(Context *c);

/* digcave */
void chunkDigCave(Chunks *chunk, Block *chunkBlockCache[16][16][16][16]);

#endif /* HEADER_CHUNKS_H */
