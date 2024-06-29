#ifndef HEADER_CHUNKS_H
#define HEADER_CHUNKS_H

#include "typedef_struct.h"

#define CHUNKS_HEIGHT   	256U	/* Height max for one chunk */
#define SUB_CHUNKS_HEIGHT   16U		/* Height max for one subchunk */

/* Number of subchunks in chunk  */
#define SUB_CHUNKS_MAX (CHUNKS_HEIGHT / SUB_CHUNKS_HEIGHT)

#define BLOCKS_PER_CHUNK   	16	/* Number of block in X */
#define CAVE_ENTRY_DEPTH	15	/* Cave entry depth */

/* Weighted noise values */
#define CONTINENTAL_WEIGHT		0.8f
#define EROSION_WEIGHT			0.7f
#define PEAKS_VALLEYS_WEIGHT	0.5f

/* Scale noise value for height */
#define SCALE_NOISE_HEIGHT		60.0f

struct s_sub_chunks {
	HashMap 		*block_map;		/* Blocks map, use HashMap API to set/get block */
};

struct s_chunks {
	SubChunks		sub_chunks[SUB_CHUNKS_MAX]; 	/* array of sub_chunks */
	u32				nb_block;						/* nb block (outdated value total of blockmap subchunk) */
	s32				x;								/* Chunk offset X can be negative  (relative to origine chunks) */	
	s32				z;								/* Chunk offset Z can be negative (relative to origine chunks) */
	suseconds_t		lastUpdate;						/* Last update time */
	PerlinData		**noiseData;					/* Continental noise value */
	u8				**perlinCave;					/* Perlin noise cave value */
	u8				**perlinTree;					/* Perlin noise tree value */
	RenderChunks 	*render;						/* RenderChunks pointer */
	u8				neighbors;						/* Neighbors */
	u8				occlusionDone;					/* Neighbors occlusion done */
	s8				biomeId;						/* Chunk biomeId */
	BiomBlock		*biomeBlock;						/* Biome block */
};

#define CHUNK_NEIGHBOR_BACK 	1U
#define CHUNK_NEIGHBOR_FRONT 	2U
#define CHUNK_NEIGHBOR_LEFT  	4U
#define CHUNK_NEIGHBOR_RIGHT	8U

#define CHUNKS_NEIGHBOR_LOADED (CHUNK_NEIGHBOR_BACK | CHUNK_NEIGHBOR_FRONT | CHUNK_NEIGHBOR_LEFT | CHUNK_NEIGHBOR_RIGHT)

/* chunks.c */
void 	chunksMapFree(void *entry);
u32 	subChunksMaxGet(Chunks *chunk);
Chunks	*getChunkAt(Context *c, s32 x, s32 z);
Chunks	*chunksLoad(Block *****chunkBlockCache, NoiseGeneration *noise, s32 chunkX, s32 chunkZ);
s32		chunksEuclideanDistanceGet(s32 camChunkX, s32 camChunkZ, s32 chunkX, s32 chunkZ);

/* load_chunk.c*/
void	unloadChunkHandler(Context *c);

/* digcave.c */
void digCaveCall(Chunks *chunk, Block *****chunkBlockCache, PerlinData **perlinVal);

#endif /* HEADER_CHUNKS_H */
