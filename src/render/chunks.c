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

/**
 * @brief BRUT fill subchunks with block
 * @param sub_chunk Subchunk pointer
 * @return size_t Number of block filled (hashmap size)
*/
size_t BRUT_fill_subchunks(SubChunks *sub_chunk, s32 **maxHeight, s32 nb)
{
	s32 startYWorld = nb * 16;
	(void)nb;

    for (s32 i = 0; i < 16; ++i) {
        for (s32 j = 0; j < 16; ++j) {
            for (s32 k = 0; k < 16; ++k) {
                if (startYWorld + j < maxHeight[i][k]) { /* To change if we are under to sea lvl */
					Block *block = ft_calloc(sizeof(Block), 1);
					if (!block) {
						ft_printf_fd(2, "Failed to allocate block\n");
						return (0);
					}
					block->x = i;
					block->y = j;
					block->z = k;
					if (startYWorld + j > (s32)((f32)maxHeight[i][k] * 0.8)) {
						block->type = DIRT;
						if (startYWorld + j == maxHeight[i][k] - 1)
							block->type = GRASS;
					} else {
						block->type = STONE;
					}
					hashmap_set_entry(sub_chunk->block_map, (BlockPos){i, j, k}, block);
				}
            }
			// startYWorld++;
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


s32 maxHeightGet(s32 **maxHeight) {
	s32 max = 0;

	for (s32 i = 0; i < 16; ++i) {
		for (s32 j = 0; j < 16; ++j) {
			if (maxHeight[i][j] > max) {
				max = maxHeight[i][j];
			}
		}
	}
	return (max);
}


/* Interpolate noise value */
f32 getInterpolatedNoise(u8 *perlinNoise, s32 x, s32 z, s32 width, s32 height) {
    s32 x0 = x % width;
    s32 x1 = (x0 + 1) % width;
    s32 z0 = z % height;
    s32 z1 = (z0 + 1) % height;

    f32 sx = x - (s32)x;
    f32 sz = z - (s32)z;

    f32 n0 = perlinNoise[z0 * width + x0] / 255.0f;
    f32 n1 = perlinNoise[z0 * width + x1] / 255.0f;
    f32 n2 = perlinNoise[z1 * width + x0] / 255.0f;
    f32 n3 = perlinNoise[z1 * width + x1] / 255.0f;

    f32 i0 = n0 + sx * (n1 - n0);
    f32 i1 = n2 + sx * (n3 - n2);

    return (i0 + sz * (i1 - i0));
}

f32 perlinNoiseHeight(Mutex *mtx, u8 *perlinNoise, s32 worldX, s32 worldZ) {
    /* Set local X and Z coordinates based on the center of the Perlin noise array */
    s32 localX = worldX + (PERLIN_NOISE_WIDTH / 2);
    s32 localZ = worldZ + (PERLIN_NOISE_HEIGHT / 2);

    /* Access the interpolated noise value */
    f32 perlinValue = getInterpolatedNoise(perlinNoise, localX, localZ, PERLIN_NOISE_WIDTH, PERLIN_NOISE_HEIGHT);
    f32 scale = 20.0f;

	(void)mtx;
    // mtx_lock(mtx);
    // ft_printf_fd(1, "Perlin of World "ORANGE"[%d][%d]"RESET"->"PURPLE"[%d][%d]"RESET" "PINK"PVal |%f|"RESET"\n", worldX, worldZ, localX, localZ, perlinValue);
    // mtx_unlock(mtx);

    return (20.0f + (perlinValue * scale));
}

/**
 * @brief Brut fill chunks with block and set his cardinal offset
 * @param chunks Chunks array pointer
*/
void BRUT_FillChunks(Mutex *mtx, u8 *perlinNoise, Chunks *chunks) {
	s32 **maxHeight = ft_calloc(sizeof(s32 *), 16);

	for (u32 y = 0; y < 16; ++y) {
		maxHeight[y] = ft_calloc(sizeof(s32), 16);
		for (u32 x = 0; x < 16; ++x) {
			s32 xWorld = (s32)localXToWorld(chunks, x);
			s32 yWorld = (s32)localZToWorld(chunks, y);
			maxHeight[y][x] = perlinNoiseHeight(mtx, perlinNoise, xWorld, yWorld);

		}
	}

	s32 chunkMaxY = maxHeightGet(maxHeight);


	for (s32 i = 0; (i * 16) < chunkMaxY; ++i) {
		chunks->sub_chunks[i].block_map = hashmap_init(HASHMAP_SIZE_1000, hashmap_entry_free);
		chunks->nb_block += BRUT_fill_subchunks(&chunks->sub_chunks[i], maxHeight, i);
		chunks->visible_block += checkHiddenBlock(chunks, i);
		// free(maxHeight[i]);
	}

	// for (s32 i = 0; (i * 16) < chunkMaxY; ++i) {
	// }

	for (u32 y = 0; y < 16; ++y) {
		free(maxHeight[y]);
	}
	free(maxHeight);
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

Chunks *chunksLoad(Mutex *mtx, u8 *perlinNoise, s32 chunkX, s32 chunkZ) {
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
	BRUT_FillChunks(mtx, perlinNoise, chunks);
	return (chunks);
}