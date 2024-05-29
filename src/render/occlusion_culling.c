#include "../../include/world.h"
#include "../../include/chunks.h"

/* Occlusion Culling Strategy */

void updateNeighbors(Block *block, Block *blockCache[16][16][16]) {
    BlockPos pos[6] = {
        {block->x, block->y, block->z + 1}, {block->x, block->y, block->z - 1},
        {block->x + 1, block->y, block->z}, {block->x - 1, block->y, block->z},
        {block->x, block->y + 1, block->z}, {block->x, block->y - 1, block->z},
    };
    // These masks are for updating the neighbors of the current block
    u8 block_masks[6] = {
        NEIGHBOR_FRONT, NEIGHBOR_BACK,
        NEIGHBOR_RIGHT, NEIGHBOR_LEFT,
        NEIGHBOR_TOP, NEIGHBOR_BOTTOM,
    };
    // These masks are for updating the current block
    u8 neighbor_masks[6] = {
        NEIGHBOR_BACK, NEIGHBOR_FRONT,
        NEIGHBOR_LEFT, NEIGHBOR_RIGHT,
        NEIGHBOR_BOTTOM, NEIGHBOR_TOP,
    };

    for (u32 i = 0; i < 6; ++i) {
        if (pos[i].x >= 0 && pos[i].x < 16 && pos[i].y >= 0 && pos[i].y < 16 && pos[i].z >= 0 && pos[i].z < 16) {
            Block *neighbor = blockCache[pos[i].x][pos[i].y][pos[i].z];
            if (neighbor != NULL) {
                neighbor->neighbors |= neighbor_masks[i];
                block->neighbors |= block_masks[i];
            }
        }
    }
}

void blockYLayerCacheLoad(Block *blockCache[16][16], SubChunks *subChunk, u32 layer) {
	for (u32 x = 0; x < 16; ++x) {
		for (u32 z = 0; z < 16; ++z) {
			blockCache[x][z] = hashmap_get(subChunk->block_map, (BlockPos){x, layer, z});
		}
	}
}

/* 
	@brief We need to update top of botSubChunk accordate with top blockCache
	@param botSubChunk the bottom subchunk to update (maybe we can give the last layer of the top subchunk loaded in cache)
	@param topBlockCache the top block cache 
*/
void updateTopBotNeighbors(SubChunks *botSubChunk, Block *topBlockCache[16][16][16]) {
	Block *botBlockCache[16][16];

	blockYLayerCacheLoad(botBlockCache, botSubChunk, 16 - 1);

	for (u32 x = 0; x < 16; ++x) {
		for (u32 z = 0; z < 16; ++z) {
			Block *botBlock = botBlockCache[x][z];
			Block *topBlock = topBlockCache[x][0][z];
			if (botBlock && topBlock) {
				botBlock->neighbors |= NEIGHBOR_TOP;
				topBlock->neighbors |= NEIGHBOR_BOTTOM;
			}
		}
	}
}

Chunks *getChunkAt(Context *c, s32 x, s32 z) {
	return (hashmap_get(c->world->chunksMap, (BlockPos){0, x, z}));
}

Block *getBlockAt(Chunks *chunk, u32 x, u32 y, u32 z, u32 subChunkID) {
	(void)y;
	// return (hashmap_get(chunk->sub_chunks[y / 16].block_map, (BlockPos){x, y % 16, z}));
	return (hashmap_get(chunk->sub_chunks[subChunkID].block_map, (BlockPos){x, y, z}));
}

u32 subChunksMaxGet(Chunks *chunk) {
	u32 subChunksMax = 0;

	while (chunk->sub_chunks[subChunksMax].block_map != NULL) {
		++subChunksMax;
	}
	--subChunksMax;
	return (subChunksMax);
}

void logBlockNeighbors(Block *block, const char *position, char *color) {
    ft_printf_fd(1, "%s Block at %s: neighbors mask = %d\n"RESET,color,  position, block->neighbors);
}

void updateChunkNeighbors(Context *c, Chunks *chunk, Block *chunkBlockCache[16][16][16][16]) {
    u32 subChunksMax = subChunksMaxGet(chunk);
	// s32 camChunkX = 0, camChunkZ = 0;
	// mtx_lock(&c->gameMtx);
	// camChunkX = c->cam.chunkPos[0];
	// camChunkZ = c->cam.chunkPos[2];
	// mtx_unlock(&c->gameMtx);
    BlockPos pos[4] = {
        {chunk->x, 0, chunk->z + 1}, // front
        {chunk->x, 0, chunk->z - 1}, // back
        {chunk->x + 1, 0, chunk->z}, // right
        {chunk->x - 1, 0, chunk->z}, // left
    };

    u8 block_masks[4] = {
        NEIGHBOR_FRONT, NEIGHBOR_BACK,
        NEIGHBOR_RIGHT, NEIGHBOR_LEFT,
    };

    u8 neighbor_masks[4] = {
        NEIGHBOR_BACK, NEIGHBOR_FRONT,
        NEIGHBOR_LEFT, NEIGHBOR_RIGHT,
    };

    for (u32 i = 0; i < 4; ++i) {
		// if (chunksEuclideanDistanceGet(camChunkX, camChunkZ, pos[i].x, pos[i].z) >= CHUNKS_LOAD_RADIUS) { continue ;}
		mtx_lock(&c->threadContext->chunkMtx);
		Chunks *neighborChunk = getChunkAt(c, pos[i].x, pos[i].z);
		mtx_unlock(&c->threadContext->chunkMtx);
		if (!neighborChunk) { continue; }
		u32 neighborSubChunksMax = subChunksMaxGet(neighborChunk);
		for (u32 subChunkID = 0; subChunkID <= subChunksMax && subChunkID <= neighborSubChunksMax; ++subChunkID) {
			for (u32 y = 0; y < 16; ++y) {
				for (u32 x = 0; x < 16; ++x) {
					for (u32 z = 0; z < 16; ++z) {
						// Block *block = getBlockAt(chunk, x, y, z, subChunkID);
						Block *block = chunkBlockCache[subChunkID][x][y][z];
						Block *neighborBlock = NULL;

						switch (i) {
							case 0: // front
								if (z == 15)
									neighborBlock = getBlockAt(neighborChunk, x, y, 0, subChunkID);
								break;
							case 1: // back
								if (z == 0)
									neighborBlock = getBlockAt(neighborChunk, x, y, 15, subChunkID);
								break;
							case 2: // right
								if (x == 15)
									neighborBlock = getBlockAt(neighborChunk, 0, y, z, subChunkID);
								break;
							case 3: // left
								if (x == 0)
									neighborBlock = getBlockAt(neighborChunk, 15, y, z, subChunkID);
								break;
						}

						if (block != NULL && neighborBlock != NULL) {
							block->neighbors |= block_masks[i];
							neighborBlock->neighbors |= neighbor_masks[i];
						}
					} /* for z */
				} 	/* for x */
			}	/* for y */
		} /* for subChunkID */
		neighborChunk->lastUpdate = get_ms_time();
    } /* for i (direction )*/
    chunk->lastUpdate = get_ms_time();
}


/* Not mandatory now, we check face instead of block, but maybe good to check only no hidden block for perf */

// s8 allNeighborsExist(Block *block) {
// 	return (block->neighbors == BLOCK_HIDDEN);
// }

// u32 checkHiddenBlock(Chunks *chunks, u32 subChunksID) {
//     s8 next = TRUE;
//     HashMap *block_map = chunks->sub_chunks[subChunksID].block_map;
//     HashMap_it it = hashmap_iterator(block_map);
//     next = hashmap_next(&it);
//     u32 nb_block = hashmap_size(block_map);

//     while (next) {
//         Block *block = (Block *)it.value;
//         if (allNeighborsExist(block)) {
//             --nb_block;
//         }
//         next = hashmap_next(&it);
//     }
//     return (nb_block);
// }



/* OLD Occlusion Culling Strategy */


/**
 * @brief Check if all neighbors of a block exist
 * @param block_map the hashmap containing all blocks
 * @param x,y,z the position of the block
 * @return TRUE if all neighbors exist, FALSE otherwise
*/
// s8 allNeighborsExist(HashMap *block_map, u32 x, u32 y, u32 z)
// {
// 	if (hashmap_get(block_map, (BlockPos){x + 1, y, z}) &&
// 		hashmap_get(block_map, (BlockPos){x - 1, y, z}) &&
// 		hashmap_get(block_map, (BlockPos){x, y + 1, z}) &&
// 		hashmap_get(block_map, (BlockPos){x, y - 1, z}) &&
// 		hashmap_get(block_map, (BlockPos){x, y, z + 1}) &&
// 		hashmap_get(block_map, (BlockPos){x, y, z - 1}))
// 	{
// 		return (TRUE);
// 	}
// 	return (FALSE);
// }

/**
 * @brief Check if a block is hidden and note it as such
 * @param chunks the chunks containing all blocks
 * @return the number of visible block
*/
// u32 checkHiddenBlock(Chunks *chunks, u32 subChunksID)
// {
//     s8 next = TRUE;
// 	HashMap *block_map = chunks->sub_chunks[subChunksID].block_map;
// 	HashMap_it it = hashmap_iterator(block_map);
// 	next = hashmap_next(&it);
// 	u32 nb_block = hashmap_size(block_map);

// 	while (next) {
// 		Block *block = (Block *)it.value;
// 		if (allNeighborsExist(block_map, block->x, block->y, block->z)) {
// 			block->flag = BLOCK_HIDDEN;
// 			--nb_block;
// 		}
// 		next = hashmap_next(&it);	
// 	}
// 	return (nb_block);
// }