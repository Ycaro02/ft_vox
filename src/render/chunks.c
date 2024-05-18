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
		free(e->value); /* free the value (allocaated ptr) */
	}
	free(e); /* free the entry t_list node */
}

Block *blockCreate(s32 x, s32 y, s32 z, s32 maxHeight, s32 startYWorld) {
	Block *block = ft_calloc(sizeof(Block), 1);
	if (!block) {
		ft_printf_fd(2, "Failed to allocate block\n");
		return (NULL);
	}

	block->x = x;
	block->y = y;
	block->z = z;
	
	s32 realY = startYWorld + y;

	// if (realY > maxHeight && realY < (s32)SEA_LEVEL) {
	// 	block->type = WATER;
	// 	return (block);
	// } 
	
	if (realY >= maxHeight && realY <= (s32)SEA_LEVEL) {
		// block->flag = BLOCK_HIDDEN;
		block->type = WATER;
		return (block);
	}

	if (realY >= (s32)((f32)maxHeight * 0.95)) {
		block->type = DIRT;
		if (realY == maxHeight - 1)
			block->type = GRASS;
	} else {
		block->type = STONE;
	}
	return (block);
}

/**
 * @brief BRUT fill subchunks with block
 * @param sub_chunk Subchunk pointer
 * @return size_t Number of block filled (hashmap size)
*/
size_t BRUT_fill_subchunks(SubChunks *sub_chunk, DebugPerlin **perlinVal, s32 nb)
{
	s32 startYWorld = nb * 16;

    for (s32 i = 0; i < 16; ++i) {
        for (s32 j = 0; j < 16; ++j) {
            for (s32 k = 0; k < 16; ++k) {
					// if (startYWorld + j < maxHeight[i][k]) {
					if (startYWorld + j < perlinVal[i][k].normalise || startYWorld + j <= (s32)SEA_LEVEL) {
						Block *block = blockCreate(i,j,k, perlinVal[i][k].normalise, startYWorld);
						hashmap_set_entry(sub_chunk->block_map, (BlockPos){i, j, k}, block);
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


/* Interpolate noise value */
f32 getInterpolatedNoise(u8 **perlinNoise, s32 x, s32 z, s32 width, s32 height, DebugPerlin *perlinVal) {
    s32 x0 = x % width;
    s32 x1 = (x0 + 1) % width;
    s32 z0 = z % height;
    s32 z1 = (z0 + 1) % height;

    f32 sx = x - (s32)x;
    f32 sz = z - (s32)z;

	perlinVal->z0 = z0;
	perlinVal->z1 = z1;
	perlinVal->x0 = x0;
	perlinVal->x1 = x1;


	perlinVal->givenX = x;
	perlinVal->givenZ = z;

    perlinVal->n0 = perlinNoise[z0][x0] / 255.0f;
    perlinVal->n1 = perlinNoise[z0][x1] / 255.0f;
    perlinVal->n2 = perlinNoise[z1][x0] / 255.0f;
    perlinVal->n3 = perlinNoise[z1][x1] / 255.0f;

    f32 i0 = perlinVal->n0 + sx * (perlinVal->n1 - perlinVal->n0);
    f32 i1 = perlinVal->n2 + sx * (perlinVal->n3 - perlinVal->n2);

    return (i0 + sz * (i1 - i0));
}

f32 perlinNoiseHeight(Mutex *mtx, u8 **perlin2D, s32 localX, s32 localZ, DebugPerlin *perlinVal) {
    (void)mtx;
    /* Access the interpolated noise value */
    perlinVal->val = getInterpolatedNoise(perlin2D, localX, localZ, PERLIN_NOISE_WIDTH, PERLIN_NOISE_HEIGHT, perlinVal);
    f32 scale = 60.0f;

    // if (perlinVal->val > 0.6 && perlinVal->val < 0.8) {
    //     scale = perlinVal->val * 100.0f;
    // } else if (perlinVal->val >= 0.8) {
    //     return (150.0f);
    // }

    perlinVal->add =  (perlinVal->val * scale);

    return ((SEA_LEVEL - PERLIN_SUB_HEIGHT) + (perlinVal->val * scale));
}
/**
 * @brief Brut fill chunks with block and set his cardinal offset
 * @param chunks Chunks array pointer
*/
void BRUT_FillChunks(Mutex *mtx, u8 **perlin2D, Chunks *chunks) {
	DebugPerlin **perlinVal = ft_calloc(sizeof(DebugPerlin *), 16);

	for (u32 x = 0; x < 16; ++x) {
		perlinVal[x] = ft_calloc(sizeof(DebugPerlin), 16);
		for (u32 z = 0; z < 16; ++z) {
		    /* Set local X and Z coordinates based on the center of the Perlin noise array */
			s32 globalX = chunks->x * 16 + x;
			s32 globalZ = chunks->z * 16 + z;
			s32 localX = globalX + (PERLIN_NOISE_WIDTH / 2);
			s32 localZ = globalZ + (PERLIN_NOISE_HEIGHT / 2);
			perlinVal[x][z].normalise = perlinNoiseHeight(mtx, perlin2D, localX, localZ, &perlinVal[x][z]);
		}
	}

	s32 chunkMaxY = maxHeightGet(perlinVal);
	if (chunkMaxY < (s32)SEA_LEVEL) {
		chunkMaxY = (s32)SEA_LEVEL;
	}


	for (s32 i = 0; (i * 16) < chunkMaxY; ++i) {
		chunks->sub_chunks[i].block_map = hashmap_init(HASHMAP_SIZE_1000, hashmap_entry_free);
		chunks->nb_block += BRUT_fill_subchunks(&chunks->sub_chunks[i], perlinVal, i);
		chunks->visible_block += checkHiddenBlock(chunks, i);
		/* SET DEBUG VALUE HERE */
		chunks->perlinVal = perlinVal;
		// free(maxHeight[i]);
	}

	// free perlinVal
	// for (u32 y = 0; y < 16; ++y) {
	// 	free(perlinVal[y]);
	// }
	// free(perlinVal);
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
			/*	Need to change world translation logic must give offset with camera position origin
				This function can be this implementation but we need to parse chunks HashMap before to
				give only chunks to render to this function
			*/
			if (block->flag != BLOCK_HIDDEN) {
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

Chunks *chunksLoad(Mutex *mtx, u8 **perlin2D, s32 chunkX, s32 chunkZ) {
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