#include "../../include/vox.h"			/* Main project header */
#include "../../include/chunks.h"		/* Main project header */
#include "../../include/perlin_noise.h"	/* Main project header */
#include "../../include/render_chunks.h"
#include "../../include/thread_load.h"

/* Basic function you can provide to hashmap_init */
void chunksMapFree(void *entry) {
	HashMap_entry *e = (HashMap_entry *)entry;
	if (e->value) {
		Chunks *chunks = (Chunks *)e->value;
		for (u32 i = 0; chunks->sub_chunks[i].block_map ; ++i) {
			hashmap_destroy(chunks->sub_chunks[i].block_map);
		}
		for (u32 i = 0; chunks->perlinVal[i]; ++i) {
			free(chunks->perlinVal[i]);
		}
		free(chunks->perlinVal);
		free(e->value); /* free the value (allocaated ptr) */
	}
	free(e); /* free the entry t_list node */
}

Block *blockCreate(s32 x, s32 y, s32 z, s32 maxHeight, s32 startYWorld) {
	Block	*block = NULL;
	s32		blockType = AIR;
	s32		realY = startYWorld + y;

	if (realY >=  maxHeight - 4 && realY < maxHeight) {
		blockType = DIRT;
		if (realY == maxHeight - 1)
			blockType = GRASS;
	} else if (realY < maxHeight) {
		blockType = STONE;
	} else if (realY <= (s32)SEA_LEVEL - 10) {
		blockType = WATER;
	} else {
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
	// blockOcllusionCulling(blockMap, block);
	return (block);
}

void initializeBlockCache(Block* blockCache[16][16][16]) {
    for (u32 x = 0; x < 16; ++x) {
        for (u32 y = 0; y < 16; ++y) {
            for (u32 z = 0; z < 16; ++z) {
                blockCache[x][y][z] = NULL;
            }
        }
    }
}

/**
 * @brief BRUT fill subchunks with block
 * @param sub_chunk Subchunk pointer
 * @return size_t Number of block filled (hashmap size)
*/
size_t BRUT_fill_subchunks(SubChunks *sub_chunk, DebugPerlin **perlinVal, s32 nb)
{
	Block *block = NULL;
	s32 startYWorld = nb * 16;
	Block *blockCache[16][16][16];

	initializeBlockCache(blockCache);

    for (s32 x = 0; x < 16; ++x) {
        for (s32 y = 0; y < 16; ++y) {
            for (s32 z = 0; z < 16; ++z) {
				if ((block = blockCreate(x ,y ,z , perlinVal[x][z].normalise, startYWorld))) {
					hashmap_set_entry(sub_chunk->block_map, (BlockPos){x, y, z}, block);
					blockCache[x][y][z] = block;
					updateNeighbors(block, blockCache);
				}
            }
        }
    }
	return (hashmap_size(sub_chunk->block_map));
}

f32 localXToWorld(Chunks *chunks, s32 x) {
	return ((f32)x + (f32)(chunks->x * 16));
}

f32 localZToWorld(Chunks *chunks, s32 z) {
	return ((f32)z + (f32)(chunks->z * 16));
}


s32 maxHeightGet(DebugPerlin **perlinVal) {
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

f32 normalisef32Tof32(f32 value, f32 start1, f32 stop1, f32 start2, f32 stop2) {
	return start2 + (stop2 - start2) * ((value - start1) / (stop1 - start1));
}

/* Interpolate noise value */
f32 normaliseNoiseGet(f32 **perlinNoise, s32 x, s32 z, s32 width, s32 height, DebugPerlin *perlinVal) {
    s32 normX = abs(x % width);
    s32 normZ = abs(z % height);

	perlinVal->z0 = normZ;
	perlinVal->x0 = normX;


	perlinVal->givenX = x;
	perlinVal->givenZ = z;

	// return (normalizeU8Tof32(perlinNoise[normX][normZ], 0, 255, -1.0f, 1.0f));
	return (perlinNoise[normX][normZ]);
}

f32 perlinNoiseHeight(Mutex *mtx, f32 **perlin2D, s32 localX, s32 localZ, DebugPerlin *perlinVal) {
    (void)mtx;
    f32 scale = 60.0f;
    /* Access the interpolated noise value */
    perlinVal->val = normaliseNoiseGet(perlin2D, localX, localZ, PERLIN_NOISE_WIDTH, PERLIN_NOISE_HEIGHT, perlinVal);

    if (perlinVal->val > 0.3 && perlinVal->val <= 0.7) {
		f32 ret = normalisef32Tof32(perlinVal->val, 0.3, 0.7, 100.0f, 150.0f);
		return (ret);

    } else if (perlinVal->val >= 0.6999f) {
        return (150.0f);
    }

    perlinVal->add =  (perlinVal->val * scale);

    return ((SEA_LEVEL) + perlinVal->add);
}


/**
 * @brief Brut fill chunks with block and set his cardinal offset
 * @param chunks Chunks array pointer
*/
void BRUT_FillChunks(Mutex *mtx, f32 **perlin2D, Chunks *chunks) {
	DebugPerlin **perlinVal = ft_calloc(sizeof(DebugPerlin *), 16 + 1);

	for (u32 x = 0; x < 16; ++x) {
		perlinVal[x] = ft_calloc(sizeof(DebugPerlin), 16);
		for (u32 z = 0; z < 16; ++z) {
		    /* Set local X and Z coordinates based on the center of the Perlin noise array */
			s32 globalX = chunks->x * 16 + x;
			s32 globalZ = chunks->z * 16 + z;
			s32 localX = globalX + (PERLIN_NOISE_WIDTH / 2);
			s32 localZ = globalZ + (PERLIN_NOISE_HEIGHT / 2);
			perlinVal[x][z].normalise = (s32)perlinNoiseHeight(mtx, perlin2D, localX, localZ, &perlinVal[x][z]);
		}
	}

	s32 chunkMaxY = maxHeightGet(perlinVal);
	if (chunkMaxY < (s32)SEA_LEVEL) {
		chunkMaxY = (s32)SEA_LEVEL;
	}


	for (s32 i = 0; (i * 16) < chunkMaxY; ++i) {
		chunks->sub_chunks[i].block_map = hashmap_init(HASHMAP_SIZE_4000, hashmap_entry_free);
		chunks->nb_block += BRUT_fill_subchunks(&chunks->sub_chunks[i], perlinVal, i);
		chunks->visible_block += checkHiddenBlock(chunks, i);
		/* SET DEBUG VALUE HERE */
		chunks->perlinVal = perlinVal;
	}
}

/**
 * @brief Get the block array object
 * @param chunks Chunks pointer (data to parse)
 * @param block_array Block array pointer (output)
 * @param chunkID Chunk ID [in]
 * @return u32 Number of visible block
*/
u32 chunksCubeGet(Chunks *chunks, RenderChunks *render)
{
    s8 next = TRUE;
	u32 idx = 0;

	for (s32 subID = 0; chunks->sub_chunks[subID].block_map != NULL; ++subID) {
		HashMap_it it = hashmap_iterator(chunks->sub_chunks[subID].block_map);
		next = hashmap_next(&it);
		while (next) {
			Block *block = (Block *)it.value;
			if (block->neighbors != BLOCK_HIDDEN && block->type != AIR) {
				render->block_array[idx][0] = (f32)block->x + (f32)(chunks->x * 16);
				render->block_array[idx][1] = (f32)block->y + (f32)(subID * 16);
				render->block_array[idx][2] = (f32)block->z + (f32)(chunks->z * 16);
				render->blockTypeID[idx] = (f32)block->type;
				// ft_printf_fd(1, "Block %d = %f\n", block->type, render->blockTypeID[idx]);
				++idx;
			}
			next = hashmap_next(&it);
		}
	}

    // ft_printf_fd(1, GREEN"Renderer Cube %u\n"RESET, idx);
    return (idx);
}

s32 getChunkID() {
	static s32 chunksID = 0;
	return (chunksID++);
}

Chunks *chunksLoad(Mutex *mtx, f32 **perlin2D, s32 chunkX, s32 chunkZ) {
	Chunks *chunks = ft_calloc(sizeof(Chunks), 1);
	if (!chunks) {
		ft_printf_fd(2, "Failed to allocate chunks\n");
		return (NULL);
	}

	mtx_lock(mtx);
	chunks->id = getChunkID();
	mtx_unlock(mtx);

	chunks->x = chunkX;
	chunks->z = chunkZ;
	BRUT_FillChunks(mtx, perlin2D, chunks);
	return (chunks);
}