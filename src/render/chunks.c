#include "../../include/vox.h"			/* Main project header */
#include "../../include/chunks.h"
#include "../../rsc/perlin_noise/include/perlin_noise.h"
#include "../../include/render_chunks.h"
#include "../../include/thread_load.h"
#include "../../include/block.h"
#include "../../include/world.h"


// s32 chunksManhattanDistanceGet(s32 camChunkX, s32 camChunkZ, s32 chunkX, s32 chunkZ) {
// 	return (abs(camChunkX - chunkX) + abs(camChunkZ - chunkZ));
// }

/**
 * @brief chunksEuclideanDistanceGet, get euclidean distance between two chunk
 * @param camChunkX, camChunkZ Camera chunk X/Z position
 * @param chunkX, chunkZ Chunk X/Z position
*/
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
			free(chunks->noiseData[i]);
		}
		free(chunks->noiseData);
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


/**
 * @brief getChunkAt, get chunk at x, z position
 * @param c Context pointer
 * @param x X position
 * @param z Z position
 * @return Chunks* Chunks pointer
*/
Chunks *getChunkAt(Context *c, s32 x, s32 z) {
	return (hashmap_get(c->world->chunksMap, (BlockPos){0, x, z}));
}

/**
 * @brief ocllusionCullingStatic, update block neighbors
 * @param chunkBlockCache Block cache, pointer on 4D array represent all block in chunk
 * @param chunk Chunk pointer
*/
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
 * @brief subchunksInit, create block in each subchunk
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

/**
 * @brief Get perlin noise max height
 * @param perlinVal PerlinData pointer
 * @return s32 Max height
*/
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


/**
 * @brief Get perlin noise height
 * @param noise Perlin noise pointer
 * @param localX Local X position
 * @param localZ Local Z position
 * @param perlinVal PerlinData pointer
*/
f32 perlinNoiseHeight(NoiseGeneration *noise, s32 localX, s32 localZ, PerlinData *perlinVal) {
    f32 scale = 60.0f;

    /* Get multiple noise values */
    f32 continentalVal = interpolateNoiseGet(noise->continental, localX, localZ, perlinVal);
    f32 erosionVal = interpolateNoiseGet(noise->erosion, localX, localZ, perlinVal);
    f32 peaksValleysVal = interpolateNoiseGet(noise->peaksValley, localX, localZ, perlinVal);

	perlinVal->valHumidity = interpolateNoiseGet(noise->humidity, localX, localZ, perlinVal);
	perlinVal->valTemperature = interpolateNoiseGet(noise->temperature, localX, localZ, perlinVal);

	perlinVal->valContinent = continentalVal;
	perlinVal->valErosion = erosionVal;
	perlinVal->valPeaksValley = peaksValleysVal;

	/* Weighted noise values */
    f32 continentalWeight = 0.8f;
    f32 erosionWeight = 0.7f;
    f32 peaksValleysWeight = 0.5f;

    /* Combined noise value */
    f32 combinedNoise = (continentalVal * continentalWeight) +
                        (erosionVal * erosionWeight) +
                        (peaksValleysVal * peaksValleysWeight);

	perlinVal->valCombined = combinedNoise;
    
	if (combinedNoise > 0.4 && combinedNoise <= 0.6) {
        return (normalisef32Tof32(combinedNoise, 0.4, 0.6, 104.0f, 160.0f));
    } else if (combinedNoise >= 0.5999f) {
        return (160.0f);
    }

    return ((f32)MIN_HEIGHT + (combinedNoise * scale));
}


/**
 * @brief perlinCaveDataGet Get perlin cave data, fill chunk->perlinCave field
 * @param chunk Chunk pointer
 * @param perlinSnakeCaveNoise Perlin snake cave noise
*/
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

/**
 * @brief Brut fill chunks with block and set his cardinal offset
 * @param chunks Chunks array pointer
*/
void chunkBuild(Block *****chunkBlockCache, NoiseGeneration *noise, Chunks *chunk) {
	PerlinData **perlinVal = malloc(sizeof(PerlinData *) * BLOCKS_PER_CHUNK);

	for (u32 x = 0; x < BLOCKS_PER_CHUNK; ++x) {
		perlinVal[x] = malloc(sizeof(PerlinData) * BLOCKS_PER_CHUNK);
		for (u32 z = 0; z < BLOCKS_PER_CHUNK; ++z) {
			s32 localX = blockLocalToPerlinPos(chunk->x, x, PERLIN_NOISE_WIDTH);
			s32 localZ = blockLocalToPerlinPos(chunk->z, z, PERLIN_NOISE_WIDTH);
			// perlinVal[x][z].normalise = (s32)perlinNoiseHeight(noise->continental, localX, localZ, &perlinVal[x][z]);
			perlinVal[x][z].normalise = (s32)perlinNoiseHeight(noise, localX, localZ, &perlinVal[x][z]);
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
		chunk->noiseData = perlinVal;
	}

	// perlinCaveDataGet(chunk, perlinSnakeCaveNoise);
	perlinCaveDataGet(chunk, noise->cave);
	digCaveCall(chunk, chunkBlockCache, perlinVal);
	occlusionCullingStatic(chunkBlockCache, chunk);
}

/**
 * @brief chunksLoad, load chunk with block (Call by workers thread)
 * @param chunkBlockCache Block cache, pointer on 4D array represent all block in chunk
 * @param noise Perlin noise pointer
 * @param chunkX, chunkZ Chunk X/Z position
 * @return Chunks* Chunks pointer
*/
Chunks *chunksLoad(Block *****chunkBlockCache, NoiseGeneration *noise, s32 chunkX, s32 chunkZ) {
	Chunks *chunks = ft_calloc(sizeof(Chunks), 1);
	if (!chunks) {
		ft_printf_fd(2, "Failed to allocate chunks\n");
		return (NULL);
	}

	chunks->x = chunkX;
	chunks->z = chunkZ;
	// chunkBuild(chunkBlockCache, perlin2D, chunks, perlinSnakeCaveNoise);
	chunkBuild(chunkBlockCache, noise, chunks);
	chunks->lastUpdate = get_ms_time();
	return (chunks);
}