#ifndef HEADER_BLOCK_H
#define HEADER_BLOCK_H

#include "typedef_struct.h"		                /* Struct typedef */

/* Neighbor flag value */
#define NEIGHBOR_BACK  	1U
#define NEIGHBOR_FRONT  2U
#define NEIGHBOR_LEFT   4U
#define NEIGHBOR_RIGHT 	8U
#define NEIGHBOR_BOTTOM 16U
#define NEIGHBOR_TOP 	32U


/* Undeground block define */
#define UNDERGROUND_FACE_NB 	9
#define TOTAL_UNDERGROUND_FACE 	(UNDERGROUND_FACE_NB * 3)


#define BLOCKPOS_CMP(a, b) (a.x == b.x && a.y == b.y && a.z == b.z)


struct s_block {
    s8 	x;          /* Block x position */
    s8 	y;          /* Block y position (height) */
    s8 	z;          /* Block z position */
    s8	type;       /* Block type */
	u8  neighbors;  /* Block flag */
	s8	biomeId;	/* Biome ID */
	/* padding */
	s8 pad1, pad2;
};


Block	*getBlockAt(Chunks *chunk, u32 x, u32 y, u32 z, u32 subChunkID);
Block	*blockCreate(PerlinData **dataNoise, s32 x, s32 y, s32 z, s32 maxHeight, s32 startYWorld);
s32		blockLocalToPerlinPos(s32 chunkOffset, s32 localPos, s32 width);

void blockLocalPosFromCam(vec3 camPos, BlockPos *blockPos);
void undergroundBlockcreate(Context *c);
void undergroundBlockFree(UndergroundBlock *udg);
void undergroundBoolUpdate(Context *c, BlockPos *blockPos, s32 *columnMaxHeight);


#endif /* HEADER_BLOCK_H */
