#ifndef HEADER_CHUNKS_H
#define HEADER_CHUNKS_H

#include "../libft/HashMap/HashMap.h"

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

/* Just define context here to avoid compilation error */
struct s_context;
typedef struct s_context Context;

typedef struct s_sub_chunks {
	HashMap 		*block_map;		/* Blocks map, use HashMap API to set/get block */
	u32				flag;			/* Sub Chunk Id and flag */
	u32				metadata;		/* Sub Chunk metadata */
} SubChunks;

typedef struct s_chunks {
	SubChunks		sub_chunks[SUB_CHUNKS_MAX]; /* array of sub_chunks */
	u32				nb_block;			/* nb block (outdated value total of blockmap subchunk) */
    s32				id;     			/* Chunk Id */
	u32				visible_block;		/* Number of visible block */
	s32				x;					/* Chunk X */	
	s32				z;					/* Chunk Z */
} Chunks;

/* Render chunks.c */
void	fillChunks(HashMap *chunksMap);
u32		chunks_cube_get(Chunks *chunks, vec3 *block_array, u32 chunkID);
void	chunksLoadArround(Context *c, s32 chunksX, s32 chunksZ, s32 radius);

#endif /* HEADER_CHUNKS_H */
