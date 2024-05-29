#include "../../include/vox.h"			/* Main project header */
#include "../../include/chunks.h"
#include "../../include/perlin_noise.h"
#include "../../include/render_chunks.h"
#include "../../include/thread_load.h"

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
		for (u32 i = 0; chunks->perlinVal[i]; ++i) {
			free(chunks->perlinVal[i]);
		}
		free(chunks->perlinVal);
		if (chunks->render) {
			renderChunkFree(chunks->render);
		}
		free(e->value); /* free the value (allocaated ptr) */
	}
	free(e); /* free the entry t_list node */
}

Block *blockCreate(s32 x, s32 y, s32 z, s32 maxHeight, s32 startYWorld) {
	Block	*block = NULL;
	s32		blockType = AIR;
	s32		realY = startYWorld + y;
	s32 	seaLevel = (s32)SEA_LEVEL - 30;


	if (realY < maxHeight - 2) {
    	blockType = STONE;
	} else if (realY <= maxHeight) {
		blockType = DIRT;
		if (realY == maxHeight) { blockType = GRASS; }
	} else if (realY >= maxHeight && realY <= seaLevel) {
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
	return (block);
}

void blockCacheInit(Block* blockCache[16][16][16]) {
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
size_t subchunksInit(Block *chunkBlockCache[16][16][16][16], Chunks *chunk, SubChunks *sub_chunk, DebugPerlin **perlinVal, s32 layer)
{
	Block *block = NULL;
	s32 startYWorld = layer * 16;

	blockCacheInit(chunkBlockCache[layer]);

    for (s32 x = 0; x < 16; ++x) {
        for (s32 y = 0; y < 16; ++y) {
            for (s32 z = 0; z < 16; ++z) {
				if ((block = blockCreate(x ,y ,z , perlinVal[x][z].normalise, startYWorld))) {
					hashmap_set_entry(sub_chunk->block_map, (BlockPos){x, y, z}, block);
					chunkBlockCache[layer][x][y][z] = block;
					updateNeighbors(block, chunkBlockCache[layer]);
				}
            }
        }
    }

	if (layer != 0) {
		updateTopBotNeighbors(&chunk->sub_chunks[layer - 1], chunkBlockCache[layer]);
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
	return (start2 + (stop2 - start2) * ((value - start1) / (stop1 - start1)));
}

f32 perlinInterpolate(f32 a, f32 b, f32 t) {
    return (a + t * (b - a));
}

f32 bilinearInterpolation(f32 q11, f32 q12, f32 q21, f32 q22, f32 x, f32 z) {
    f32 r1 = perlinInterpolate(q11, q21, x);
    f32 r2 = perlinInterpolate(q12, q22, x);
    return perlinInterpolate(r1, r2, z);
}

f32 getInterpolatedPerlinNoise(f32 **perlinNoise, f32 x, f32 z, f32 scale, s32 width, s32 height, DebugPerlin *perlinVal) {
    f32 scaledX = (fabs(x) / scale);
    f32 scaledZ = (fabs(z) / scale);

    s32 x0 = scaledX;
    s32 x1 = x0 + 1;
    s32 z0 = scaledZ;
    s32 z1 = z0 + 1;

    if (x1 >= width) x1 = x0;
    if (z1 >= height) z1 = z0;

	/* set Perlin Debug val here */
	perlinVal->x0 = x0;
	perlinVal->z0 = z0;

    f32 q11 = perlinNoise[x0 % width][z0 % height];
    f32 q12 = perlinNoise[x0 % width][z1 % height];
    f32 q21 = perlinNoise[x1 % width][z0 % height];
    f32 q22 = perlinNoise[x1 % width][z1 % height];

    f32 tx = scaledX - x0;
    f32 tz = scaledZ - z0;

    return bilinearInterpolation(q11, q12, q21, q22, tx, tz);
}
/* Interpolate noise value */
f32 normaliseNoiseGet(f32 **perlinNoise, s32 x, s32 z, DebugPerlin *perlinVal) {
    // s32 normX = abs(x % PERLIN_NOISE_WIDTH);
    // s32 normZ = abs(z % PERLIN_NOISE_HEIGHT);

	/* set Perlin Debug val here */
	perlinVal->givenX = x;
	perlinVal->givenZ = z;

	// return (perlinNoise[normX][normZ]);
	// return (getInterpolatedPerlinNoise(perlinNoise, x, z, 8.0f, PERLIN_NOISE_WIDTH, PERLIN_NOISE_HEIGHT, perlinVal));
	return (getInterpolatedPerlinNoise(perlinNoise, x, z, 4.0f, PERLIN_NOISE_WIDTH, PERLIN_NOISE_HEIGHT, perlinVal));
}

f32 perlinNoiseHeight(Mutex *mtx, f32 **perlin2D, s32 localX, s32 localZ, DebugPerlin *perlinVal) {
    (void)mtx;
    f32 scale = 60.0f;
    /* Access the interpolated noise value */
    perlinVal->val = normaliseNoiseGet(perlin2D, localX, localZ, perlinVal);

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
void chunkBuild(Block *chunkBlockCache[16][16][16][16], Mutex *mtx, f32 **perlin2D, Chunks *chunks) {
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
		chunks->nb_block += subchunksInit(chunkBlockCache, chunks, &chunks->sub_chunks[i], perlinVal, i);
		// chunks->visible_block += checkHiddenBlock(chunks, i);
		/* SET DEBUG VALUE HERE */
		chunks->perlinVal = perlinVal;
	}
}

Chunks *chunksLoad(Block *chunkBlockCache[16][16][16][16], Mutex *mtx, f32 **perlin2D, s32 chunkX, s32 chunkZ) {
	Chunks *chunks = ft_calloc(sizeof(Chunks), 1);
	if (!chunks) {
		ft_printf_fd(2, "Failed to allocate chunks\n");
		return (NULL);
	}

	chunks->x = chunkX;
	chunks->z = chunkZ;
	chunkBuild(chunkBlockCache, mtx, perlin2D, chunks);
	chunks->lastUpdate = get_ms_time();
	return (chunks);
}