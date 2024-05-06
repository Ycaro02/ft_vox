#include "../../include/vox.h"			/* Main project header */
#include "../../include/chunks.h"		/* Main project header */

/**
 * @brief BRUT fill subchunks with block
 * @param sub_chunk Subchunk pointer
 * @return size_t Number of block filled (hashmap size)
*/
size_t BRUT_fill_subchunks(SubChunks *sub_chunk)
{
    for (s32 i = 0; i < 16; ++i) {
        for (s32 j = 0; j < 16; ++j) {
            for (s32 k = 0; k < 16; ++k) {
                Block *block = ft_calloc(sizeof(Block), 1);
				if (!block) {
					ft_printf_fd(2, "Failed to allocate block\n");
					return (0);
				}
                block->x = i;
                block->y = j;
                block->z = k;
                block->type = STONE;
				hashmap_set_entry(sub_chunk->block_map, (BlockPos){i, j, k}, block);
            }
        }
    }
	return (hashmap_size(sub_chunk->block_map));
}

/**
 * @brief Brut fill chunks with block and set his cardinal offset
 * @param chunks Chunks array pointer
*/
void BRUT_FillChunks(Chunks *chunks) {

	for (u32 i = 0; i < SUBCHUNKS_DISPLAY; ++i) {
		chunks->nb_block += BRUT_fill_subchunks(&chunks->sub_chunks[i]);
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
u32 chunks_cube_get(Chunks *chunks, vec3 *block_array, u32 chunkID)
{
    s8 next = TRUE;
	u32 idx = 0;

	(void)chunkID;

	for (u32 subID = 0; subID < SUBCHUNKS_DISPLAY; ++subID) {
		HashMap_it it = hashmap_iterator(chunks->sub_chunks[subID].block_map);
		next = hashmap_next(&it);
		while (next) {
			Block *block = (Block *)it.value;
			/*	Need to change world translation logic must give offset with camera position origin
				This function can be this implementation but we need to parse chunks HashMap before to
				give only chunks to render to this function
			*/
			if (block->flag != BLOCK_HIDDEN) {
				block_array[idx][0] = (f32)block->x + (f32)(chunks->x * 16);
				block_array[idx][1] = (f32)block->y + (f32)(subID * SUB_CHUNKS_HEIGHT);
				block_array[idx][2] = (f32)block->z + (f32)(chunks->z * 16);
				// if (idx == 0) {
				// 	ft_printf_fd(1, "First block chunkId:%d x:%f y:%f z:%f\n", chunks->id, block_array[idx][0], block_array[idx][1], block_array[idx][2]);
				// }
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



Chunks *chunksLoad(s32 x, s32 z) {
	Chunks *chunks = ft_calloc(sizeof(Chunks), 1);
	if (!chunks) {
		ft_printf_fd(2, "Failed to allocate chunks\n");
		return (NULL);
	}

	chunks->id = getChunkID();
	chunks->x = x;
	chunks->z = z;

	ft_printf_fd(1, ORANGE"Chunk i:|%d|"RESET""CYAN"x:[%d] z:[%d]"RESET"\n", chunks->id, chunks->x, chunks->z);
	/* need to loop here to create all subchunks */
	chunks->sub_chunks[0].block_map = hashmap_init(HASHMAP_SIZE_1000, hashmap_entry_free);
	// chunks->sub_chunks[1].block_map = hashmap_init(HASHMAP_SIZE_1000, hashmap_entry_free);
	BRUT_FillChunks(chunks);
	return (chunks);
}

/**
 * @brief Fill all chunks, call brut fill chunk on all chunkks and set hashmap
 * @param c Context pointer
*/
void fillChunks(HashMap *chunksMap) {
		static vec2_s32 off[] = {
		{0, 0},\
		{1, 0},\
		{-1, 0},\
		{0, 1},\
		{0, -1},\
		{1, 1},\
		{-1, -1},\
		{1, -1},\
		{-1, 1},\
	};
	if (!chunksMap) {
		ft_printf_fd(2, "ChunksMap is NULL in fillChunks\n");
		return ;
	}

	for (s32 i = 0; i < TEST_CHUNK_MAX; i++) {
		Chunks *chunks = chunksLoad(off[i][0], off[i][1]);
		hashmap_set_entry(chunksMap, (BlockPos){0, chunks->x, chunks->z}, chunks);
	}

}

/**
 * @brief Scan the environment to load chunks arround the camera
 * @param c Context pointer
 * @param curr_x Player's x position
 * @param curr_z Player's z position
 * @param radius The radius around the player to scan
*/
void chunksLoadArround(Context *c, s32 chunksX, s32 chunksZ, s32 radius) {
	(void)chunksX;
	(void)chunksZ;

	s32  currentX = c->cam.chunkPos[0];
	s32  currentZ = c->cam.chunkPos[2];
	for (s32 i = -radius; i < radius; ++i) {
		for (s32 j = -radius; j < radius; ++j) {
			BlockPos pos = { .x = 0, .y = currentX + i, .z = currentZ + j};
			Chunks *chunks = hashmap_get(c->world->chunksMap, pos);
			if (!chunks) {
				// ft_printf_fd(1, RED"Chunk not exist REALX:%d x: %d z: %d\n"RESET, pos.x, pos.y, pos.z);
				Chunks *newChunks = chunksLoad(pos.y, pos.z);
				hashmap_set_entry(c->world->chunksMap, pos, newChunks);
				// ft_printf_fd(1, ORANGE"Chunk Created x: %d z: %d\n"RESET, pos.y, pos.z);
			}
		}
	}


}