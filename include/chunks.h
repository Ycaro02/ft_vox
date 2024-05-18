#ifndef HEADER_CHUNKS_H
#define HEADER_CHUNKS_H

#include "world.h"

#define CHUNKS_HEIGHT   256U	/* that will be 256 */
#define CHUNKS_WIDTH    16U
#define CHUNKS_DEPTH    16U

/* Number of maximum block in chunks */
#define MAX_CHUNKS_BLOCK (CHUNKS_HEIGHT * CHUNKS_WIDTH * CHUNKS_DEPTH) 

#define SUB_CHUNKS_HEIGHT   16U
#define SUB_CHUNKS_WIDTH    16U
#define SUB_CHUNKS_DEPTH    16U

/* Number of maximum block in sub chunks */
#define MAX_SUB_CHUNKS_BLOCK (SUB_CHUNKS_HEIGHT * SUB_CHUNKS_WIDTH * SUB_CHUNKS_DEPTH)

/* Number of maximum cub chunks in chunks */
#define SUB_CHUNKS_MAX (CHUNKS_HEIGHT / SUB_CHUNKS_HEIGHT)

/* Just need to change it to display/fill  more subchunks */
#define SUBCHUNKS_DISPLAY 1U

typedef struct s_sub_chunks {
	HashMap 		*block_map;		/* Blocks map, use HashMap API to set/get block */
} SubChunks;

typedef struct s_debug_perlin {
	f32 n0, n1, n2, n3;
	s32 z0, z1, x0, x1;
	f32 val;
	f32 add;
	s32 normalise;
	s32 givenX, givenZ;
} DebugPerlin;


typedef struct s_chunks {
	SubChunks		sub_chunks[SUB_CHUNKS_MAX]; /* array of sub_chunks */
	u32				nb_block;			/* nb block (outdated value total of blockmap subchunk) */
    s32				id;     			/* Chunk Id */
	u32				visible_block;		/* Number of visible block */
	s32				x;					/* Chunk offset X can be negative  (relative to origine chunks) */	
	s32				z;					/* Chunk offset Z can be negative (relative to origine chunks) */
	DebugPerlin		**perlinVal;		/* Perlin noise value */
} Chunks;


void chunksMapFree(void *entry);

/* chunks */
Chunks *chunksLoad(Mutex *mtx, u8 **perlin2D, s32 chunkX, s32 chunkZ);;

#endif /* HEADER_CHUNKS_H */
