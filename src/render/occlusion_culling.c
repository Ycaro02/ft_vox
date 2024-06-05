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
    u8 blockMasks[6] = {
        NEIGHBOR_FRONT, NEIGHBOR_BACK,
        NEIGHBOR_RIGHT, NEIGHBOR_LEFT,
        NEIGHBOR_TOP, NEIGHBOR_BOTTOM,
    };
    // These masks are for updating the current block
    u8 neighborMasks[6] = {
        NEIGHBOR_BACK, NEIGHBOR_FRONT,
        NEIGHBOR_LEFT, NEIGHBOR_RIGHT,
        NEIGHBOR_BOTTOM, NEIGHBOR_TOP,
    };

    for (u32 i = 0; i < 6; ++i) {
        if (pos[i].x >= 0 && pos[i].x < 16 && pos[i].y >= 0 && pos[i].y < 16 && pos[i].z >= 0 && pos[i].z < 16) {
            Block *neighbor = blockCache[pos[i].x][pos[i].y][pos[i].z];
            if (neighbor != NULL) {
                neighbor->neighbors |= neighborMasks[i];
                block->neighbors |= blockMasks[i];
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

void chunkNeighborsGet(Context *c, Chunks *chunk, Chunks *neighborChunksCache[4]) {
	BlockPos pos[4] = {
        {chunk->x, 0, chunk->z + 1}, // front
        {chunk->x, 0, chunk->z - 1}, // back
        {chunk->x + 1, 0, chunk->z}, // right
        {chunk->x - 1, 0, chunk->z}, // left
    };

	for (u32 i = 0; i < 4; ++i) {
		mtx_lock(&c->threadContext->chunkMtx);
		neighborChunksCache[i] = getChunkAt(c, pos[i].x, pos[i].z);
		mtx_unlock(&c->threadContext->chunkMtx);
	}
}

void chunkNeighborMaskUpdate(Context *c, Chunks *chunk) {
	BlockPos pos[4] = {
		{chunk->x, 0, chunk->z + 1}, // front
        {chunk->x, 0, chunk->z - 1}, // back
        {chunk->x + 1, 0, chunk->z}, // right
        {chunk->x - 1, 0, chunk->z}, // left
    };

	u8 chunkMask[4] = {
		NEIGHBOR_FRONT, NEIGHBOR_BACK,
		NEIGHBOR_RIGHT, NEIGHBOR_LEFT,
	};

	Chunks *neightborsChunk = NULL;

	for (u32 i = 0; i < 4; ++i) {
		if (!(chunk->neighbors & chunkMask[i])) {
			mtx_lock(&c->threadContext->chunkMtx);
			neightborsChunk = getChunkAt(c, pos[i].x, pos[i].z);
			mtx_unlock(&c->threadContext->chunkMtx);
		}
		if (neightborsChunk) {
			chunk->neighbors |= chunkMask[i];
		}
	}

}

void updateChunkNeighbors(Context *c, Chunks *chunk, Block *chunkBlockCache[16][16][16][16], Chunks *neighborChunksCache[4]) {
    u32 subChunksMax = subChunksMaxGet(chunk);
	// s32 camChunkX = 0, camChunkZ = 0;
	// mtx_lock(&c->gameMtx);
	// camChunkX = c->cam.chunkPos[0];
	// camChunkZ = c->cam.chunkPos[2];
	// mtx_unlock(&c->gameMtx);
    // BlockPos pos[4] = {
    //     {chunk->x, 0, chunk->z + 1}, // front
    //     {chunk->x, 0, chunk->z - 1}, // back
    //     {chunk->x + 1, 0, chunk->z}, // right
    //     {chunk->x - 1, 0, chunk->z}, // left
    // };
	(void)c;

    u8 blockMasks[4] = {
        NEIGHBOR_FRONT, NEIGHBOR_BACK,
        NEIGHBOR_RIGHT, NEIGHBOR_LEFT,
    };

    u8 neighborMasks[4] = {
        NEIGHBOR_BACK, NEIGHBOR_FRONT,
        NEIGHBOR_LEFT, NEIGHBOR_RIGHT,
    };

    for (u32 i = 0; i < 4; ++i) {
		Chunks *neighborChunk = neighborChunksCache[i];
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
							case CHUNK_FRONT: // front
								if (z == 15)
									neighborBlock = getBlockAt(neighborChunk, x, y, 0, subChunkID);
								break;
							case CHUNK_BACK: // back
								if (z == 0)
									neighborBlock = getBlockAt(neighborChunk, x, y, 15, subChunkID);
								break;
							case CHUNK_RIGHT: // right
								if (x == 15)
									neighborBlock = getBlockAt(neighborChunk, 0, y, z, subChunkID);
								break;
							case CHUNK_LEFT: // left
								if (x == 0)
									neighborBlock = getBlockAt(neighborChunk, 15, y, z, subChunkID);
								break;
						}

						if (block != NULL && neighborBlock != NULL) {
							block->neighbors |= blockMasks[i];
							neighborBlock->neighbors |= neighborMasks[i];
						}
					} /* for z */
				} 	/* for x */
			}	/* for y */
		} /* for subChunkID */
		neighborChunk->lastUpdate = get_ms_time();
    } /* for i (direction )*/
    chunk->lastUpdate = get_ms_time();
}


s8 blockVisibleFromCam(vec3 blockPos, vec3 faceNormal, Camera *camera) {
    vec3 toCamera = {
        camera->position[0] - blockPos[0],
        camera->position[1] - blockPos[1],
        camera->position[2] - blockPos[2]
    };

    /* Normalise toCamera vector */
    float length = sqrt(toCamera[0] * toCamera[0] + toCamera[1] * toCamera[1] + toCamera[2] * toCamera[2]);
    toCamera[0] /= length;
    toCamera[1] /= length;
    toCamera[2] /= length;

   /* Dot product between the toCamera vector and the face normal */
    float dotProduct = camera->viewVector[0] * faceNormal[0] + camera->viewVector[1] * faceNormal[1] + camera->viewVector[2] * faceNormal[2];

	/* if the dot product is negative, the face is visible */
    return (dotProduct < 0);
}

void updateVisibleFaces(Chunks *chunk, Camera *camera) {
	s8 next = FALSE;
    for (u32 subChunkID = 0; chunk->sub_chunks[subChunkID].block_map ; ++subChunkID) {
        SubChunks *subChunk = &chunk->sub_chunks[subChunkID];
        HashMap_it it = hashmap_iterator(subChunk->block_map);
        while ((next = hashmap_next(&it))) {
            Block		*block = it.value;
            vec3		blockPos = { block->x * (16 * chunk->x), block->y + subChunkID * 16, block->z * (16 * chunk->z) };

            u8 visibleFaces = 0;
			/* Visible mask */
            vec3 faceNormals[6] = {
                {0, 0, 1}, {0, 0, -1},
                {1, 0, 0}, {-1, 0, 0},
                {0, 1, 0}, {0, -1, 0}
            };

            for (int i = 0; i < 6; ++i) {
                if (blockVisibleFromCam(blockPos, faceNormals[i], camera)) {
                    visibleFaces |= (1 << i);
                }
            }
            block->visibleFromCam = visibleFaces;
        }
    }
	// chunks->lastUpdate = get_ms_time();
}





// void updateVisibleFaces(Chunks *chunk, Camera *camera) {
//     s8 next = FALSE;

//     // Directions de balayage basées sur le vecteur de vue de la caméra
//     int xStart, xEnd, xStep;
//     int yStart, yEnd, yStep;
//     int zStart, zEnd, zStep;

//     if (camera->viewVector.x > 0) {
//         xStart = 0; xEnd = 16; xStep = 1;
//     } else {
//         xStart = 15; xEnd = -1; xStep = -1;
//     }

//     if (camera->viewVector.y > 0) {
//         yStart = 0; yEnd = 16; yStep = 1;
//     } else {
//         yStart = 15; yEnd = -1; yStep = -1;
//     }

//     if (camera->viewVector.z > 0) {
//         zStart = 0; zEnd = 16; zStep = 1;
//     } else {
//         zStart = 15; zEnd = -1; zStep = -1;
//     }

//     for (u32 subChunkID = 0; subChunkID < SUB_CHUNKS_MAX; ++subChunkID) {
//         SubChunks *subChunk = &chunk->sub_chunks[subChunkID];
//         HashMap_it it = hashmap_iterator(subChunk->block_map);

//         while ((next = hashmap_next(&it))) {
//             Block *block = it.value;
//             BlockPos blockPos = { block->x * (16 * chunk->x), block->y + subChunkID * 16, block->z * (16 * chunk->z) };

//             // Masques de visibilité pour les 6 faces
//             u8 visibleFaces = 0;
//             vec3 faceNormals[6] = {
//                 {0, 0, 1}, {0, 0, -1},
//                 {1, 0, 0}, {-1, 0, 0},
//                 {0, 1, 0}, {0, -1, 0}
//             };

//             for (int i = 0; i < 6; ++i) {
//                 if (isFaceVisible(blockPos, faceNormals[i], camera)) {
//                     visibleFaces |= (1 << i);

//                     // Stop the iteration if this block is visible and we are moving in that direction
//                     if ((i == 0 && zStep == -1) || (i == 1 && zStep == 1) ||
//                         (i == 2 && xStep == -1) || (i == 3 && xStep == 1) ||
//                         (i == 4 && yStep == -1) || (i == 5 && yStep == 1)) {
//                         break;
//                     }
//                 }
//             }

//             block->visibleFromCam = visibleFaces;
//         }
//     }
// }
