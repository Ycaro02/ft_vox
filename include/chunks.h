#ifndef HEADER_CHUNKS_H
#define HEADER_CHUNKS_H

#include "world.h"

#define CHUNKS_HEIGHT   	256U	/* that will be 256 */
#define SUB_CHUNKS_HEIGHT   16U

/* Number of subchunks in chunk  */
#define SUB_CHUNKS_MAX (CHUNKS_HEIGHT / SUB_CHUNKS_HEIGHT)

typedef struct s_sub_chunks {
	HashMap 		*block_map;		/* Blocks map, use HashMap API to set/get block */
	s32				y;
} SubChunks;

typedef struct s_debug_perlin {
	s32 z0, x0;
	f32 val;
	f32 add;
	s32 normalise;
	s32 givenX, givenZ;
} DebugPerlin;

typedef struct s_chunks {
	SubChunks		sub_chunks[SUB_CHUNKS_MAX]; /* array of sub_chunks */
	u32				nb_block;					/* nb block (outdated value total of blockmap subchunk) */
    s32				id;     					/* Chunk Id */
	u32				visible_block;				/* Number of visible block */
	s32				x;							/* Chunk offset X can be negative  (relative to origine chunks) */	
	s32				z;							/* Chunk offset Z can be negative (relative to origine chunks) */
	DebugPerlin		**perlinVal;				/* Perlin noise value */
} Chunks;


void chunksMapFree(void *entry);

/* chunks */
Chunks *chunksLoad(Mutex *mtx, f32 **perlin2D, s32 chunkX, s32 chunkZ);;

#endif /* HEADER_CHUNKS_H */
