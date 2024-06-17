#include "../../include/vox.h"			/* Main project header */
#include "../../include/chunks.h"
#include "../../include/perlin_noise.h"
#include "../../include/render_chunks.h"
#include "../../include/thread_load.h"
#include "../../include/block.h"
#include "../../include/world.h"


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
		for (u32 i = 0; i < BLOCKS_PER_CHUNK; ++i) {
			free(chunks->perlinVal[i]);
		}
		free(chunks->perlinVal);
		/* cave */
		if (chunks->perlinCave) {
			for (u32 i = 0; i < BLOCKS_PER_CHUNK; ++i) {
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

void occlusionCullingStatic(Block *****chunkBlockCache, Chunks *chunk) {
	
	s32 i = 0;
	while (chunk->sub_chunks[i].block_map) {
		for (s32 x = 0; x < BLOCKS_PER_CHUNK; ++x) {
			for (s32 y = 0; y < BLOCKS_PER_CHUNK; ++y) {
				for (s32 z = 0; z < BLOCKS_PER_CHUNK; ++z) {
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
size_t subchunksInit(Block *****chunkBlockCache, SubChunks *sub_chunk, PerlinData **perlinVal, s32 layer)
{
	Block *block = NULL;
	s32 startYWorld = layer * BLOCKS_PER_CHUNK;

    for (s32 x = 0; x < BLOCKS_PER_CHUNK; ++x) {
        for (s32 y = 0; y < BLOCKS_PER_CHUNK; ++y) {
            for (s32 z = 0; z < BLOCKS_PER_CHUNK; ++z) {
				if ((block = blockCreate(x ,y ,z , perlinVal[x][z].normalise, startYWorld))) {
					hashmap_set_entry(sub_chunk->block_map, (BlockPos){x, y, z}, block);
					chunkBlockCache[layer][x][y][z] = block;
				}
            }
        }
    }
	return (hashmap_size(sub_chunk->block_map));
}

s32 maxHeightGet(PerlinData **perlinVal) {
	s32 max = 0;

	for (s32 i = 0; i < BLOCKS_PER_CHUNK; ++i) {
		for (s32 j = 0; j < BLOCKS_PER_CHUNK; ++j) {
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

    return ((MIN_HEIGHT) + perlinVal->add);
}


void perlinCaveDataGet(Chunks *chunk, u8 **perlinSnakeCaveNoise) {
	u8 **caveData = malloc(sizeof(u8 *) * BLOCKS_PER_CHUNK);
	s32 width = PERLIN_SNAKE_WIDTH;
	s32 height = PERLIN_SNAKE_HEIGHT;

	for (u32 x = 0; x < BLOCKS_PER_CHUNK; ++x) {
		caveData[x] = malloc(sizeof(u8) * BLOCKS_PER_CHUNK);
		for (u32 z = 0; z < BLOCKS_PER_CHUNK; ++z) {
			s32 localX = blockLocalToPerlinPos(chunk->x, x, width);
			s32 localZ = blockLocalToPerlinPos(chunk->z, z, height);
			caveData[x][z] = perlinSnakeCaveNoise[abs(localX % width)][abs(localZ % height)];
		}
	}
	chunk->perlinCave = caveData;
}




s8 isCaveValue(u8 val) {
	return (val == PATH_VAL || val == ENTRY_EXIT_VAL);
}

s8 digHeightGuard(s32 startY) {
	return (startY <= 0 || (startY >= SEA_LEVEL - 2  && startY <= SEA_LEVEL + 2));
}


void caveDigXAxis(Chunks *chunk, Block *****chunkBlockCache, u8 **perlinCave, s32 startX, s32 startY, s32 startZ) {

	if (digHeightGuard(startY)) { return; }

	if (startX >= 0 && startX < BLOCKS_PER_CHUNK && isCaveValue(perlinCave[startX][startZ])) {
		hashmap_remove_entry(chunk->sub_chunks[startY / BLOCKS_PER_CHUNK].block_map, (BlockPos){startX, startY % BLOCKS_PER_CHUNK, startZ}, HASHMAP_FREE_DATA);
		chunkBlockCache[startY / BLOCKS_PER_CHUNK][startX][startY % BLOCKS_PER_CHUNK][startZ] = NULL;
	}
}

void caveDigZAxis(Chunks *chunk, Block *****chunkBlockCache, u8 **perlinCave, s32 startX, s32 startY, s32 startZ) {
	
	if (digHeightGuard(startY)) { return; }

	if (startZ >= 0 && startZ < BLOCKS_PER_CHUNK && isCaveValue(perlinCave[startX][startZ])) {
		hashmap_remove_entry(chunk->sub_chunks[startY / BLOCKS_PER_CHUNK].block_map, (BlockPos){startX, startY % BLOCKS_PER_CHUNK, startZ}, HASHMAP_FREE_DATA);
		chunkBlockCache[startY / BLOCKS_PER_CHUNK][startX][startY % BLOCKS_PER_CHUNK][startZ] = NULL;
	}

}

void caveEntryMark(Chunks *chunk, Block *****chunkBlockCache, s32 startX, s32 startY, s32 startZ) {
	s32 maxDepth = startY - CAVE_ENTRY_DEPTH;

	for (s32 tmpY = startY; tmpY > maxDepth; --tmpY) {
		s32 subChunkId = tmpY / BLOCKS_PER_CHUNK;
		if (chunkBlockCache[subChunkId][startX][tmpY % BLOCKS_PER_CHUNK][startZ]) {
			hashmap_remove_entry(chunk->sub_chunks[subChunkId].block_map, (BlockPos){startX, tmpY % BLOCKS_PER_CHUNK, startZ}, HASHMAP_FREE_DATA);
			chunkBlockCache[subChunkId][startX][tmpY % BLOCKS_PER_CHUNK][startZ] = NULL;
		}
	}
}


void digCaveCall(Chunks *chunk, Block *****chunkBlockCache, PerlinData **perlinVal) {
    s32 startX = 0, startY = 0, startZ = 0, maxDepth = 0;

	for (s32 x = 0; x < BLOCKS_PER_CHUNK; ++x) {
		for (s32 z = 0; z < BLOCKS_PER_CHUNK; ++z) {
			if (chunk->perlinCave[x][z] == ENTRY_EXIT_VAL) {
				// ft_printf_fd(1, "Start cave at [%d][%d][%d]\n", x, perlinVal[x][z].normalise, z);
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
				if (startY > 65 ) { startY = 65; }
				startZ = z;
				// maxDepth = (perlinVal[x][z].normalise - CAVE_ENTRY_DEPTH) / 4;
				maxDepth = 4;
				for (s32 depth = 0 ; depth < maxDepth; depth++) {
					for (s32 horizontalDepth = 0; horizontalDepth < BLOCKS_PER_CHUNK; horizontalDepth++) {
						caveDigZAxis(chunk, chunkBlockCache, chunk->perlinCave, startX, startY-depth, startZ - horizontalDepth);
						caveDigZAxis(chunk, chunkBlockCache, chunk->perlinCave, startX, startY-depth, startZ + horizontalDepth);
						caveDigXAxis(chunk, chunkBlockCache, chunk->perlinCave, startX - horizontalDepth, startY-depth, startZ);
						caveDigXAxis(chunk, chunkBlockCache, chunk->perlinCave, startX + horizontalDepth, startY-depth, startZ);
					}
				}
			}
		}
	}

}


/**
 * @brief Brut fill chunks with block and set his cardinal offset
 * @param chunks Chunks array pointer
*/
void chunkBuild(Block *****chunkBlockCache, f32 **perlin2D, Chunks *chunk, u8 **perlinSnakeCaveNoise) {
	PerlinData **perlinVal = malloc(sizeof(PerlinData *) * BLOCKS_PER_CHUNK);

	for (u32 x = 0; x < BLOCKS_PER_CHUNK; ++x) {
		perlinVal[x] = malloc(sizeof(PerlinData) * BLOCKS_PER_CHUNK);
		for (u32 z = 0; z < BLOCKS_PER_CHUNK; ++z) {
			s32 localX = blockLocalToPerlinPos(chunk->x, x, PERLIN_NOISE_WIDTH);
			s32 localZ = blockLocalToPerlinPos(chunk->z, z, PERLIN_NOISE_WIDTH);
			perlinVal[x][z].normalise = (s32)perlinNoiseHeight(perlin2D, localX, localZ, &perlinVal[x][z]);
		}
	}

	s32 chunkMaxY = maxHeightGet(perlinVal);
	if (chunkMaxY < (s32)MIN_HEIGHT) {
		chunkMaxY = (s32)MIN_HEIGHT;
	}

	for (s32 i = 0; (i * BLOCKS_PER_CHUNK) < chunkMaxY; ++i) {
		chunk->sub_chunks[i].block_map = hashmap_init(HASHMAP_SIZE_4000, hashmap_entry_free);
		if (!chunk->sub_chunks[i].block_map) {
			ft_printf_fd(2, "Failed to allocate hashmap\n");
			return;
		}
		chunk->nb_block += subchunksInit(chunkBlockCache, &chunk->sub_chunks[i], perlinVal, i);
		chunk->perlinVal = perlinVal;
	}

	perlinCaveDataGet(chunk, perlinSnakeCaveNoise);

	digCaveCall(chunk, chunkBlockCache, perlinVal);
	occlusionCullingStatic(chunkBlockCache, chunk);
}

Chunks *chunksLoad(Block *****chunkBlockCache, f32 **perlin2D, s32 chunkX, s32 chunkZ, u8 **perlinSnakeCaveNoise) {
	Chunks *chunks = ft_calloc(sizeof(Chunks), 1);
	if (!chunks) {
		ft_printf_fd(2, "Failed to allocate chunks\n");
		return (NULL);
	}

	chunks->x = chunkX;
	chunks->z = chunkZ;
	chunkBuild(chunkBlockCache, perlin2D, chunks, perlinSnakeCaveNoise);
	chunks->lastUpdate = get_ms_time();
	return (chunks);
}