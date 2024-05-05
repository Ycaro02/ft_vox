#ifndef HEADER_CHUNKS_H
#define HEADER_CHUNKS_H

#include "../libft/hashMap/hashMap.h"

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
typedef struct s_context t_context;

typedef struct s_sub_chunks {
	hashMap 		*block_map;		/* Blocks map, use hashMap API to set/get block */
	u32				flag;			/* Sub Chunk Id and flag */
	u32				metadata;		/* Sub Chunk metadata */
} t_sub_chunks;

typedef struct s_chunks {
	t_sub_chunks	sub_chunks[SUB_CHUNKS_MAX]; /* array of sub_chunks */
	u32				nb_block;			/* nb block (outdated value total of blockmap subchunk) */
    s32				id;     			/* Chunk Id */
	u32				visible_block;		/* Number of visible block */
	s32				x;					/* Chunk X */	
	s32				z;					/* Chunk Z */
	// t_cardinal_offset offset;			/* offset to get the cardinal sub_chunks */
} t_chunks;

/* Render chunks.c */
void	fillChunks(hashMap *chunksMap);
u32		chunks_cube_get(t_chunks *chunks, vec3 *block_array, u32 chunkID);
void	chunksLoadArround(t_context *c, s32 chunksX, s32 chunksZ, s32 radius);

#endif /* HEADER_CHUNKS_H */
