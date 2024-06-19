#include "../../include/vox.h"			/* Main project header */
#include "../../include/chunks.h"
#include "../../rsc/perlin_noise/include/perlin_noise.h"
#include "../../include/render_chunks.h"
#include "../../include/thread_load.h"

FT_INLINE s8 isCaveValue(u8 val) {
	return (val == PATH_VAL || val == ENTRY_EXIT_VAL);
}

FT_INLINE s8 digHeightGuard(s32 startY) {
	return (startY <= 0 || (startY >= SEA_LEVEL - 2  && startY <= SEA_LEVEL + 2));
}

FT_INLINE s8 digAxisGuard(s32 value) {
	return (value < 0 || value >= BLOCKS_PER_CHUNK);
}

static void caveDig(Chunks *chunk, Block *****chunkBlockCache, u8 **perlinCave, s32 startX, s32 startY, s32 startZ) {

	s8 dataRemoved = -1;

	if (digHeightGuard(startY) || digAxisGuard(startX) || digAxisGuard(startZ)\
		|| !isCaveValue(perlinCave[startX][startZ])) {
		return ;
	}
	dataRemoved = hashmap_remove_entry(chunk->sub_chunks[startY / BLOCKS_PER_CHUNK].block_map, (BlockPos){startX, startY % BLOCKS_PER_CHUNK, startZ}, HASHMAP_FREE_DATA);
	if (dataRemoved == HASHMAP_DATA_REMOVED) {
		chunkBlockCache[startY / BLOCKS_PER_CHUNK][startX][startY % BLOCKS_PER_CHUNK][startZ] = NULL;
	}
}

static void caveEntryMark(Chunks *chunk, Block *****chunkBlockCache, s32 startX, s32 startY, s32 startZ) {
	s32 maxDepth = startY - CAVE_ENTRY_DEPTH;
	s32 subChunkId = 0;

	for (s32 tmpY = startY; tmpY > maxDepth; --tmpY) {
		subChunkId = tmpY / BLOCKS_PER_CHUNK;
		if (chunkBlockCache[subChunkId][startX][tmpY % BLOCKS_PER_CHUNK][startZ]) {
			hashmap_remove_entry(chunk->sub_chunks[subChunkId].block_map, (BlockPos){startX, tmpY % BLOCKS_PER_CHUNK, startZ}, HASHMAP_FREE_DATA);
			chunkBlockCache[subChunkId][startX][tmpY % BLOCKS_PER_CHUNK][startZ] = NULL;
		}
	}
}


void digCaveCall(Chunks *chunk, Block *****chunkBlockCache, PerlinData **perlinVal) {
    s32 startX = 0, startY = 0, startZ = 0, maxDepth = 0, verticalDepth = 0;

	for (s32 x = 0; x < BLOCKS_PER_CHUNK; ++x) {
		for (s32 z = 0; z < BLOCKS_PER_CHUNK; ++z) {
			if (chunk->perlinCave[x][z] == ENTRY_EXIT_VAL) {
				startX = x;
				startY = perlinVal[x][z].normalise; // Assuming the height map can provide an initial Y value
				startZ = z;
				if (startY < 80) {
					caveEntryMark(chunk, chunkBlockCache, startX, startY, startZ);
				}
			}
		}
	}

	for (s32 x = 0; x < BLOCKS_PER_CHUNK; ++x) {
		for (s32 z = 0; z < BLOCKS_PER_CHUNK; ++z) {
			if (isCaveValue(chunk->perlinCave[x][z])) {
				startX = x;
				startY = perlinVal[x][z].normalise - CAVE_ENTRY_DEPTH; /* 80 - 15 = 65*/
				if (startY > 65 ) { 
					startY = 65;
				}
				startZ = z;
				maxDepth = 4;
				for (s32 depth = 0 ; depth < maxDepth; depth++) {
					verticalDepth = startY - depth;
					for (s32 horizontalDepth = 0; horizontalDepth < BLOCKS_PER_CHUNK; horizontalDepth++) {
						caveDig(chunk, chunkBlockCache, chunk->perlinCave, startX, verticalDepth, startZ - horizontalDepth);
						caveDig(chunk, chunkBlockCache, chunk->perlinCave, startX, verticalDepth, startZ + horizontalDepth);
						caveDig(chunk, chunkBlockCache, chunk->perlinCave, startX - horizontalDepth, verticalDepth, startZ);
						caveDig(chunk, chunkBlockCache, chunk->perlinCave, startX + horizontalDepth, verticalDepth, startZ);
					}
				}
			}
		}
	}
}
