#include "../../include/vox.h"			/* Main project header */
#include "../../include/chunks.h"
#include "../../rsc/perlin_noise/include/perlin_noise.h"
#include "../../include/render_chunks.h"
#include "../../include/thread_load.h"
#include "../../include/block.h"
#include "../../include/world.h"
#include "../../include/biome.h"


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
		free(chunks->biomeBlock);
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
	return (hashmap_get(c->world->chunksMap, CHUNKS_MAP_ID_GET(x, z)));
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
size_t subchunksInit(Block *****chunkBlockCache, SubChunks *sub_chunk, PerlinData **perlinVal, s32 layer, BiomBlock *biomeBlock)
{
	Block *block = NULL;
	s32 startYWorld = layer * BLOCKS_PER_CHUNK;
	
    for (s32 x = 0; x < BLOCKS_PER_CHUNK; ++x) {
        for (s32 y = 0; y < BLOCKS_PER_CHUNK; ++y) {
            for (s32 z = 0; z < BLOCKS_PER_CHUNK; ++z) {
				if ((block = blockCreate(x ,y ,z , perlinVal[x][z].normalise, startYWorld, biomeBlock))) {
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
void perlinValueFill(NoiseGeneration *noise, s32 localX, s32 localZ, PerlinData *perlinVal) {
    /* Combined noise value */
    f32 combinedNoise = 0.0f;

	s32 h = PERLIN_NOISE_HEIGHT;
	s32 w = PERLIN_NOISE_WIDTH;

    /* Get multiple noise values */
    perlinVal->valContinent = interpolateNoiseGet(noise->continental, localX, localZ, 8.0f, w, h, perlinVal);
    perlinVal->valErosion  = interpolateNoiseGet(noise->erosion, localX, localZ, 8.0f, w, h, perlinVal);
    perlinVal->valPeaksValley = interpolateNoiseGet(noise->peaksValley, localX, localZ, 8.0f, w, h, perlinVal);
	perlinVal->valHumidity = interpolateNoiseGet(noise->humidity, localX, localZ, PERLIN_BIOME_SCALE, w, h, perlinVal);
	perlinVal->valTemperature = interpolateNoiseGet(noise->temperature, localX, localZ, PERLIN_BIOME_SCALE, w, h, perlinVal);
	
	combinedNoise = (perlinVal->valContinent * CONTINENTAL_WEIGHT) +
					(perlinVal->valErosion * EROSION_WEIGHT) +
					(perlinVal->valPeaksValley * PEAKS_VALLEYS_WEIGHT);

	perlinVal->valCombined = combinedNoise;
    
	if (combinedNoise >= 0.4 && combinedNoise <= 0.6) {
        perlinVal->normalise = normalisef32Tof32(combinedNoise, 0.4, 0.6, 104.0f, 160.0f);
    } else if (combinedNoise >= 0.5999f) {
        perlinVal->normalise = 160.0f;
    } else {
    	perlinVal->normalise = (s32)roundf((f32)MIN_HEIGHT + (combinedNoise * SCALE_NOISE_HEIGHT));
		if (perlinVal->normalise < 20) {
			perlinVal->normalise = abs(perlinVal->normalise);
		}
		if (perlinVal->normalise > 160) {
			perlinVal->normalise = 160;
		}
	}
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
			s32 scaleX = abs((localX / 2) % width);
			s32 scaleZ = abs((localZ / 2) % height);
			caveData[x][z] = perlinSnakeCaveNoise[scaleX][scaleZ];
		}
	}
	chunk->perlinCave = caveData;
}


s8 blockExist(Block *****chunkBlockCache, BlockPos pos) {
	s32 subId = pos.y / BLOCKS_PER_CHUNK;
	s32 localY = pos.y % BLOCKS_PER_CHUNK;

	if (pos.x < 0 || pos.x >= BLOCKS_PER_CHUNK || pos.z < 0 || pos.z >= BLOCKS_PER_CHUNK) {
		return (FALSE);
	}
	if (subId < 0 || subId >= 16) {
		return (FALSE);
	}
	return (chunkBlockCache[subId][pos.x][localY][pos.z] != NULL);
}

/* Do the average between all block column BiomeId to determine the entire chunk biome Id*/
s8 chunkBiomeIdGet(PerlinData **perlinVal) {
	f32 count = 0.0f;
	f32 totalHumidity = 0.0f;
	f32 totalTemperature = 0.0f;


	for (s32 x = 0; x < BLOCKS_PER_CHUNK; ++x) {
		for (s32 z = 0; z < BLOCKS_PER_CHUNK; ++z) {
			totalHumidity += perlinVal[x][z].valHumidity;
			totalTemperature += perlinVal[x][z].valTemperature;
			count += 1.0f;
		}
	}
	f32 averageHumidity = totalHumidity / count;
	f32 averageTemperature = totalTemperature / count;
	s8 biomeId = blockBiomeIdGet(averageTemperature, averageHumidity);
	return (biomeId);
}

void flowerCreate(Block *****chunkBlockCache,Chunks * chunk, s32 x, s32 y, s32 z, s32 flowerId) {
	// static s8 flowerTypeArray[] = {
	// 	FLOWER_DANDELION,
	// 	FLOWER_BLUE_ORCHID,
	// 	FLOWER_AZURE,	
	// 	FLOWER_ALLIUM,
	// 	FLOWER_CORNFLOWER,
	// 	FLOWER_CHERRY,
	// 	FLOWER_WHITE_TULIP,
	// 	FLOWER_RED_TULIP,
	// 	FLOWER_PINK_TULIP,
	// 	FLOWER_DAYSIE,
	// 	FLOWER_POPPY,
	// 	FLOWER_LILY,
	// 	PLANT_FERN,
	// 	PLANT_GRASS,
	// 	MUSHROOM_RED,
	// 	MUSHROOM_BROWN,
	// };

	s32 localY = y % 16;
	s32 subId = y / 16;
	if (blockExist(chunkBlockCache, (BlockPos){x, y - 1, z}) && ((y - 1) % 16) < 15) {
		// Block *block = basicBlockCreate(x, localY, z, flowerTypeArray[flowerId]);
		Block *block = basicBlockCreate(x, localY, z, flowerId);
		hashmap_set_entry(chunk->sub_chunks[subId].block_map, (BlockPos){x, localY, z}, block);
		chunkBlockCache[subId][x][localY][z] = block;
	}
}

void flowerTreeGeneration(Block *****chunkBlockCache, Chunks *chunk, PerlinData **perlinVal)
{
		/*	Tree generation 3 and 4 cause of cubeLeaf size is 3 
		To refactor
		- We can generate array of random value before to avoid multiple call to srand()/rand()
	*/
	u32 chunkSeed = ((u32)chunk->x ^ (u32)chunk->z) ^ 65536;
    srand(chunkSeed);
	s32 spawnRate = rand() % 40;

    for (u32 x = 3; x < BLOCKS_PER_CHUNK; x += 4) {
        for (u32 z = 3; z < BLOCKS_PER_CHUNK; z += 4) {
            s32 y = perlinVal[x][z].normalise + 1;

			if (x + 3 > BLOCKS_PER_CHUNK || z + 3 > BLOCKS_PER_CHUNK) continue;
            if (y > 100 || y <= SEA_LEVEL + 5) break;

      		if (blockExist(chunkBlockCache, (BlockPos){x, perlinVal[x][z].normalise, z})) {
				if ((rand() % 100) < spawnRate) {
					s32 treeId = (abs(chunk->x) + abs(chunk->z)) % chunk->biomeBlock->treeMax;
                	treeCreate(chunkBlockCache, chunk, (BlockPos){x, y, z},\
					 chunk->biomeBlock->tree[treeId]);
					//  (abs(chunk->x) + abs(chunk->z)) % TREE_IDX_MAX);
				} else {
					s32 flowerId = (abs(chunk->x) + abs(chunk->z)) % chunk->biomeBlock->flowersMax;
					flowerCreate(chunkBlockCache, chunk, x, y, z,\
					 chunk->biomeBlock->flowers[flowerId]);
					//  (abs(chunk->x) + abs(chunk->z)) % FLOWER_IDX_MAX);
				}
            }
        }
    }
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
			perlinValueFill(noise, localX, localZ, &perlinVal[x][z]);
		}
	}
	
	if (!(chunk->biomeBlock = malloc(sizeof(BiomBlock)))) {
		ft_printf_fd(2, "Failed to allocate biomeBlock\n");
		return;
	}

	chunk->biomeId = chunkBiomeIdGet(perlinVal);
	biomDetection(chunk->biomeBlock, chunk->biomeId);
	chunk->noiseData = perlinVal;

	s32 chunkMaxY = maxHeightGet(perlinVal);
	if (chunkMaxY < (s32)MIN_HEIGHT) {
		chunkMaxY = (s32)MIN_HEIGHT;
	}

	for (s32 i = 0; (i * BLOCKS_PER_CHUNK) <= chunkMaxY; ++i) {
		chunk->sub_chunks[i].block_map = hashmap_init(HASHMAP_SIZE_4000, hashmap_entry_free);
		if (!chunk->sub_chunks[i].block_map) {
			ft_printf_fd(2, "Failed to allocate hashmap\n");
			return;
		}
		chunk->nb_block += subchunksInit(chunkBlockCache, &chunk->sub_chunks[i], chunk->noiseData, i, chunk->biomeBlock);
	}

	perlinCaveDataGet(chunk, noise->cave);
	digCaveCall(chunk, chunkBlockCache, perlinVal);


	flowerTreeGeneration(chunkBlockCache, chunk, perlinVal);

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