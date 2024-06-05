#include "../../include/vox.h"			/* Main project header */
#include "../../include/chunks.h"
#include "../../include/perlin_noise.h"
#include "../../include/render_chunks.h"
#include "../../include/thread_load.h"

void digCrossInSurface(Chunks *chunk, Block *chunkBlockCache[16][16][16][16], s32 maxSubChunk) {
	s32 maxY = 15;
	for (s32 x = 0; x < 4; ++x) {
		for (s32 z = 0; z < 4; ++z) {
			if (chunkBlockCache[maxSubChunk][x][maxY][z]) {
				hashmap_remove_entry(chunk->sub_chunks[maxSubChunk].block_map, (BlockPos){x, maxY, z}, HASHMAP_FREE_DATA);
				chunkBlockCache[maxSubChunk][x][maxY][z] = NULL;
			} else {
				maxY--;
				x = 0;
				z = 0;
			} 
		}
		maxY = 15;
	}
}

void chunkDigCave(Chunks *chunk, Block *chunkBlockCache[16][16][16][16]) {
	s32 depth[16][16] = {0};
	for (s32 x = 0; x < 16; ++x) {
		for (s32 z = 0; z < 16; ++z) {
			depth[x][z] = (s32)chunk->perlinCave[x][z].normalise * 0.5;
		}
	}

	s32 y = 15;
	s32 maxSubChunk = subChunksMaxGet(chunk);
	s32 subID = maxSubChunk;
	
	digCrossInSurface(chunk, chunkBlockCache, maxSubChunk);

	while (subID > 0) {
		for (s32 x = 0; x < 16; ++x) {
			for (s32 z = 0; z < 16; ++z) {
				if (subID == 0 && y == 0) {break ;}
				if (y < 0) {break ;}
				if (chunkBlockCache[subID][x][y][z]) {
					hashmap_remove_entry(chunk->sub_chunks[subID].block_map, (BlockPos){x, y, z}, HASHMAP_FREE_DATA);
					chunkBlockCache[subID][x][y][z] = NULL;
					// ft_printf_fd(1, "Cave dig remove block at %d %d %d\n", x, y, z);
				}
				y--;
			}
			y = 15;
		}
		--subID;
	}
}
