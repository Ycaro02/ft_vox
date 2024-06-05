#include "../../include/vox.h"			/* Main project header */
#include "../../include/chunks.h"
#include "../../include/perlin_noise.h"
#include "../../include/render_chunks.h"
#include "../../include/thread_load.h"


void blockCacheInit(Block* blockCache[16][16][16]) {
    for (u32 x = 0; x < 16; ++x) {
        for (u32 y = 0; y < 16; ++y) {
            for (u32 z = 0; z < 16; ++z) {
                blockCache[x][y][z] = NULL;
            }
        }
    }
}

/* Set local X and Z coordinates based on the center of the Perlin noise array */
s32 blockLocalToPerlinPos(s32 chunkOffset, s32 localPos) {
	return ((chunkOffset * 16 + localPos) + (PERLIN_NOISE_WIDTH / 2));
}


Block *blockCreate(s32 x, s32 y, s32 z, s32 maxHeight, s32 startYWorld, f32 **perlinCaveNoise, Chunks *chunk) {
    Block   *block = NULL;
    s32     blockType = AIR;
    s32     realY = startYWorld + y;
    s32     seaLevel = (s32)SEA_LEVEL - 30;

	(void)perlinCaveNoise, (void)chunk;

	if (realY < maxHeight - 2) {
		blockType = STONE;
	} 
	else if (realY <= seaLevel && realY > maxHeight) {
		blockType = WATER;
	} 
	else if (realY <= maxHeight) {
		blockType = DIRT;
		if (realY == maxHeight || realY == maxHeight - 1) { blockType = GRASS;}
	} 
	else {
		return (NULL);
	}

    if (!(block = malloc(sizeof(Block)))) {
        ft_printf_fd(2, "Failed to allocate block\n");
        return (NULL);
    }
    block->type = blockType;
    block->x = x;
    block->y = y;
    block->z = z;
    block->neighbors = 0;
    return (block);
}


// s32 chunksManhattanDistanceGet(s32 camChunkX, s32 camChunkZ, s32 chunkX, s32 chunkZ) {
// 	return (abs(camChunkX - chunkX) + abs(camChunkZ - chunkZ));
// }

s32 chunksEuclideanDistanceGet(s32 camChunkX, s32 camChunkZ, s32 chunkX, s32 chunkZ) {
    return ((s32)floor(sqrt(pow(camChunkX - chunkX, 2) + pow(camChunkZ - chunkZ, 2))));
}

/* Basic function you can provide to hashmap_init */
void chunksMapFree(void *entry) {
	HashMap_entry *e = (HashMap_entry *)entry;
	if (e->value) {
		Chunks *chunks = (Chunks *)e->value;
		for (u32 i = 0; chunks->sub_chunks[i].block_map ; ++i) {
			hashmap_destroy(chunks->sub_chunks[i].block_map);
		}
		for (u32 i = 0; i < 16; ++i) {
			free(chunks->perlinVal[i]);
		}
		free(chunks->perlinVal);
		/* cave */
		if (chunks->perlinCave) {
			for (u32 i = 0; i < 16; ++i) {
				free(chunks->perlinCave[i]);
			}
			free(chunks->perlinCave);
		}
		/* cave end */
		if (chunks->render) {
			renderChunkFree(chunks->render);
		}
		free(e->value); /* free the value (allocaated ptr) */
	}
	free(e); /* free the entry t_list node */
}

Chunks *getChunkAt(Context *c, s32 x, s32 z) {
	return (hashmap_get(c->world->chunksMap, (BlockPos){0, x, z}));
}

void occlusionCullingStatic(Block *chunkBlockCache[16][16][16][16], Chunks *chunk) {
	
	s32 i = 0;
	while (chunk->sub_chunks[i].block_map) {
		for (s32 x = 0; x < 16; ++x) {
			for (s32 y = 0; y < 16; ++y) {
				for (s32 z = 0; z < 16; ++z) {
					if (chunkBlockCache[i][x][y][z]) {
						updateNeighbors(chunkBlockCache[i][x][y][z], chunkBlockCache[i]);
					}
				}
			}
		}
		if (i != 0) {
			updateTopBotNeighbors(&chunk->sub_chunks[i - 1], chunkBlockCache[i]);
		}
		++i;
	}
}

/**
 * @brief BRUT fill subchunks with block
 * @param sub_chunk Subchunk pointer
 * @return size_t Number of block filled (hashmap size)
*/
size_t subchunksInit(Block *chunkBlockCache[16][16][16][16], Chunks *chunk, SubChunks *sub_chunk, PerlinData **perlinVal, s32 layer, f32 **perlinCaveNoise)
{
	Block *block = NULL;
	s32 startYWorld = layer * 16;

	blockCacheInit(chunkBlockCache[layer]);

    for (s32 x = 0; x < 16; ++x) {
        for (s32 y = 0; y < 16; ++y) {
            for (s32 z = 0; z < 16; ++z) {
				if ((block = blockCreate(x ,y ,z , perlinVal[x][z].normalise, startYWorld, perlinCaveNoise, chunk))) {
					hashmap_set_entry(sub_chunk->block_map, (BlockPos){x, y, z}, block);
					chunkBlockCache[layer][x][y][z] = block;
					// updateNeighbors(block, chunkBlockCache[layer]);
				}
            }
        }
    }

	(void)chunk;
	// if (layer != 0) {
	// 	updateTopBotNeighbors(&chunk->sub_chunks[layer - 1], chunkBlockCache[layer]);
	// }


	return (hashmap_size(sub_chunk->block_map));
}

s32 maxHeightGet(PerlinData **perlinVal) {
	s32 max = 0;

	for (s32 i = 0; i < 16; ++i) {
		for (s32 j = 0; j < 16; ++j) {
			if (perlinVal[i][j].normalise > (s32)max) {
				max = perlinVal[i][j].normalise;
			}
		}
	}
	return (max);
}


f32 perlinNoiseHeight(f32 **perlin2D, s32 localX, s32 localZ, PerlinData *perlinVal) {
    f32 scale = 60.0f;
    /* Access the interpolated noise value */
    perlinVal->val = normaliseNoiseGet(perlin2D, localX, localZ, perlinVal);

    if (perlinVal->val > 0.3 && perlinVal->val <= 0.4) {
		f32 ret = normalisef32Tof32(perlinVal->val, 0.3, 0.4, 100.0f, 150.0f);
		return (ret);

    } else if (perlinVal->val >= 0.3999f) {
        return (150.0f);
    }

    perlinVal->add = (perlinVal->val * scale);

    return ((SEA_LEVEL) + perlinVal->add);
}


void perlinCaveDataGet(Chunks *chunk, f32 **perlinCaveNoise) {
	PerlinData **caveData = malloc(sizeof(PerlinData *) * 16);
	for (u32 x = 0; x < 16; ++x) {
		caveData[x] = malloc(sizeof(PerlinData) * 16);
		for (u32 z = 0; z < 16; ++z) {
			s32 localX = blockLocalToPerlinPos(chunk->x, x);
			s32 localZ = blockLocalToPerlinPos(chunk->z, z);
			caveData[x][z].normalise = (s32)perlinNoiseHeight(perlinCaveNoise, localX, localZ, &caveData[x][z]);
		}
	}
	chunk->perlinCave = caveData;
}


/**
 * @brief Brut fill chunks with block and set his cardinal offset
 * @param chunks Chunks array pointer
*/
void chunkBuild(Block *chunkBlockCache[16][16][16][16], Mutex *mtx, f32 **perlin2D, Chunks *chunk, f32 **perlinCaveNoise) {
	PerlinData **perlinVal = malloc(sizeof(PerlinData *) * 16);

	(void)mtx;
	for (u32 x = 0; x < 16; ++x) {
		perlinVal[x] = malloc(sizeof(PerlinData) * 16);
		for (u32 z = 0; z < 16; ++z) {
			s32 localX = blockLocalToPerlinPos(chunk->x, x);
			s32 localZ = blockLocalToPerlinPos(chunk->z, z);
			perlinVal[x][z].normalise = (s32)perlinNoiseHeight(perlin2D, localX, localZ, &perlinVal[x][z]);
		}
	}

	s32 chunkMaxY = maxHeightGet(perlinVal);
	if (chunkMaxY < (s32)SEA_LEVEL) {
		chunkMaxY = (s32)SEA_LEVEL;
	}

	for (s32 i = 0; (i * 16) < chunkMaxY; ++i) {
		chunk->sub_chunks[i].block_map = hashmap_init(HASHMAP_SIZE_4000, hashmap_entry_free);
		chunk->nb_block += subchunksInit(chunkBlockCache, chunk, &chunk->sub_chunks[i], perlinVal, i, perlinCaveNoise);
		chunk->perlinVal = perlinVal;
	}

	/* 
		for cave genetaion select if chunkMaxY is lower than 4 or 5
		Check value in PerlinCaveNoise array if value is lower than 0.3
		Then dig the chunk with cave follow the perlinCaveNoise value
	*/
	perlinCaveDataGet(chunk, perlinCaveNoise);
	// if (chunkMaxY <= (s32)SEA_LEVEL && chunk->perlinCave[0][0].val < 0.3 && chunk->perlinCave[0][0].val > 0.2) {
	// 	// ft_printf_fd(1, "Cave generation\n");
	// 	chunkDigCave(chunk, chunkBlockCache);
	// }
	occlusionCullingStatic(chunkBlockCache, chunk);

}

Chunks *chunksLoad(Block *chunkBlockCache[16][16][16][16], Mutex *mtx, f32 **perlin2D, s32 chunkX, s32 chunkZ, f32 **perlinCaveNoise) {
	Chunks *chunks = ft_calloc(sizeof(Chunks), 1);
	if (!chunks) {
		ft_printf_fd(2, "Failed to allocate chunks\n");
		return (NULL);
	}

	chunks->x = chunkX;
	chunks->z = chunkZ;
	chunkBuild(chunkBlockCache, mtx, perlin2D, chunks, perlinCaveNoise);
	chunks->lastUpdate = get_ms_time();
	return (chunks);
}