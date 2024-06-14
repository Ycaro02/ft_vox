#ifndef HEADER_CHUNKS_H
#define HEADER_CHUNKS_H

#include "world.h"

#define CHUNKS_HEIGHT   	256U	/* Height max for one chunk */
#define SUB_CHUNKS_HEIGHT   16U		/* Height max for one subchunk */

/* Number of subchunks in chunk  */
#define SUB_CHUNKS_MAX (CHUNKS_HEIGHT / SUB_CHUNKS_HEIGHT)

#define CHUNKS_NB_BLOCK   	16	/* Number of block in X */
#define CAVE_ENTRY_DEPTH	15	/* Cave entry depth */

typedef struct s_render_chunks	RenderChunks;
typedef struct s_perlin_data 	PerlinData;
typedef struct s_block 			Block;

typedef struct s_sub_chunks {
	HashMap 		*block_map;		/* Blocks map, use HashMap API to set/get block */
} SubChunks;

typedef struct s_chunks {
	SubChunks		sub_chunks[SUB_CHUNKS_MAX]; /* array of sub_chunks */
	u32				nb_block;					/* nb block (outdated value total of blockmap subchunk) */
	s32				x;							/* Chunk offset X can be negative  (relative to origine chunks) */	
	s32				z;							/* Chunk offset Z can be negative (relative to origine chunks) */
	suseconds_t		lastUpdate;					/* Last update time */
	PerlinData		**perlinVal;				/* Perlin noise value */
	u8				**perlinCave;				/* Perlin noise cave value */
	RenderChunks 	*render;					/* RenderChunks pointer */
	u8				neighbors;					/* Neighbors */
	u8				occlusionDone;				/* Neighbors occlusion done */
} Chunks;

#define CHUNK_NEIGHBOR_BACK 	1U
#define CHUNK_NEIGHBOR_FRONT 	2U
#define CHUNK_NEIGHBOR_LEFT  	4U
#define CHUNK_NEIGHBOR_RIGHT	8U

#define CHUNKS_NEIGHBOR_LOADED (CHUNK_NEIGHBOR_BACK | CHUNK_NEIGHBOR_FRONT | CHUNK_NEIGHBOR_LEFT | CHUNK_NEIGHBOR_RIGHT)

void 	chunksMapFree(void *entry);
Chunks	*getChunkAt(Context *c, s32 x, s32 z);
u32 	subChunksMaxGet(Chunks *chunk);
/* chunks */
Chunks *chunksLoad(Block *****chunkBlockCache, f32 **perlin2D, s32 chunkX, s32 chunkZ, u8 **perlinSnakeCaveNoise);
s32 chunksEuclideanDistanceGet(s32 camChunkX, s32 camChunkZ, s32 chunkX, s32 chunkZ);
void unloadChunkHandler(Context *c);

/* digcave */
// void chunkDigCave(Chunks *chunk, Block *chunkBlockCache[16][16][16][16]);

#endif /* HEADER_CHUNKS_H */
