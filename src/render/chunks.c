#include "../../include/vox.h"			/* Main project header */
#include "../../include/chunks.h"		/* Main project header */
#include "../../include/perlin_noise.h"	/* Main project header */
#include "../../include/render_chunks.h"

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
                if (startYWorld + j < maxHeight[i][k]) {
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

// Génère la hauteur du terrain pour une coordonnée mondiale donnée en utilisant le bruit de Perlin
float perlinNoiseHeight(u8 *perlinNoise, int worldX, int worldZ) {
    // Déterminez les coordonnées locales par rapport au centre du bruit de Perlin
    int localX = worldX + (PERLIN_NOISE_WIDTH / 2);
    int localZ = worldZ + (PERLIN_NOISE_HEIGHT / 2);
    
	// ft_printf_fd(1, ORANGE"World: [%d][%d] Local: [%d][%d]\n"RESET,worldX,worldZ,localX,localZ);

	int idx = (localX + localZ) % PERLIN_ARRAY_SIZE;
    // Lire la valeur de bruit de Perlin à partir du tableau de valeurs normalisées
    float perlinValue = (float)perlinNoise[idx] / 255.0f;

    // Échelle pour ajuster la hauteur du terrain en fonction de la taille du monde
    float scale = 100.0f;

	// ft_printf_fd(1, ORANGE"World: [%d][%d] Idx: [%d] "RESET""GREEN"Height: %f\n"RESET,worldX,worldZ,idx,(perlinValue * scale));
    // Retourne la valeur de bruit de Perlin comme hauteur du terrain
    return ((perlinValue * scale) - 40.0f);
    // return ((perlinValue * scale));
}


/**
 * @brief Brut fill chunks with block and set his cardinal offset
 * @param chunks Chunks array pointer
*/
void BRUT_FillChunks(Context *c, Chunks *chunks) {
	s32 **maxHeight = ft_calloc(sizeof(s32 *), 16);

	for (u32 y = 0; y < 16; ++y) {
		maxHeight[y] = ft_calloc(sizeof(s32), 16);
		for (u32 x = 0; x < 16; ++x) {
			s32 xWorld = (s32)localXToWorld(chunks, x);
			s32 yWorld = (s32)localZToWorld(chunks, y);
			// s32 idx = ((yWorld * 16) + xWorld + (1024 * 1024)) % (1024 * 1024);
			// maxHeight[y][x] = 30 + ((s32)(c->perlinNoise[idx]) / 4);

			maxHeight[y][x] = perlinNoiseHeight(c->perlinNoise, xWorld, yWorld);

			// ft_printf_fd(1, "max [%d][%d], %d\n", y,x, maxHeight[y][x]);
		}
	}

	s32 chunkMaxY = maxHeightGet(maxHeight);


	for (s32 i = 0; (i * 16) < chunkMaxY; ++i) {
		ft_printf_fd(1, "Subchunk hashmap %d created, max: %d\n", i, chunkMaxY);
		chunks->sub_chunks[i].block_map = hashmap_init(HASHMAP_SIZE_1000, hashmap_entry_free);
	}

	for (s32 i = 0; (i * 16) < chunkMaxY; ++i) {
		chunks->nb_block += BRUT_fill_subchunks(&chunks->sub_chunks[i], maxHeight, i);
		chunks->visible_block += checkHiddenBlock(chunks, i);
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

    ft_printf_fd(1, GREEN"Renderer Cube %u\n"RESET, idx);
    return (idx);
}

s32 getChunkID() {
	static s32 chunksID = 0;
	return (chunksID++);
}

Chunks *chunksLoad(Context *c, s32 chunkX, s32 chunkZ) {
	Chunks *chunks = ft_calloc(sizeof(Chunks), 1);
	if (!chunks) {
		ft_printf_fd(2, "Failed to allocate chunks\n");
		return (NULL);
	}


	chunks->id = getChunkID();
	chunks->x = chunkX;
	chunks->z = chunkZ;

	ft_printf_fd(1, ORANGE"Chunk i:|%d|"RESET""CYAN"x:[%d] z:[%d]"RESET"\n", chunks->id, chunks->x, chunks->z);
	/* need to loop here to create all subchunks */
	// chunks->sub_chunks[0].block_map = hashmap_init(HASHMAP_SIZE_1000, hashmap_entry_free);
	// chunks->sub_chunks[1].block_map = hashmap_init(HASHMAP_SIZE_1000, hashmap_entry_free);
	BRUT_FillChunks(c, chunks);
	return (chunks);
}

/**
 * @brief Scan the environment to load chunks arround the camera
 * @param c Context pointer
 * @param curr_x Player's x position
 * @param curr_z Player's z position
 * @param radius The radius around the player to scan
*/
void chunksLoadArround(Context *c, s32 radius) {
	s32  currentX = c->cam.chunkPos[0];
	s32  currentZ = c->cam.chunkPos[2];
	for (s32 i = -radius; i < radius; ++i) {
		for (s32 j = -radius; j < radius; ++j) {
			BlockPos pos = CHUNKS_MAP_ID_GET(currentX + i, currentZ + j);
			Chunks *chunks = hashmap_get(c->world->chunksMap, pos);
			if (!chunks) {
				// ft_printf_fd(1, RED"Chunk not exist REALX:%d x: %d z: %d\n"RESET, pos.x, pos.y, pos.z);
				ft_printf_fd(1, PINK"Before chunks load %d %d\n"RESET, pos.y, pos.z);

				Chunks *newChunks = chunksLoad(c, pos.y, pos.z);
				hashmap_set_entry(c->world->chunksMap, pos, newChunks);
				// ft_printf_fd(1, ORANGE"Chunk Created x: %d z: %d\n"RESET, pos.y, pos.z);
			}
		}
	}
}
