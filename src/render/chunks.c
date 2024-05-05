#include "../../include/vox.h"			/* Main project header */
#include "../../include/chunks.h"		/* Main project header */

/**
 * @brief BRUT fill subchunks with block
 * @param sub_chunk Subchunk pointer
 * @return size_t Number of block filled (hashmap size)
*/
size_t BRUT_fill_subchunks(t_sub_chunks *sub_chunk)
{
    for (u32 i = 0; i < SUB_CHUNKS_WIDTH; ++i) {
        for (u32 j = 0; j < SUB_CHUNKS_HEIGHT; ++j) {
            for (u32 k = 0; k < SUB_CHUNKS_DEPTH; ++k) {
                t_block *block = ft_calloc(sizeof(t_block), 1);
				if (!block) {
					ft_printf_fd(2, "Failed to allocate block\n");
					return (0);
				}
                block->x = i;
                block->y = j;
                block->z = k;
                block->type = STONE;
				hashmap_set_entry(sub_chunk->block_map, (t_block_pos){i, j, k}, block);
            }
        }
    }
	return (hashmap_size(sub_chunk->block_map));
}

/**
 * @brief Brut fill chunks with block and set his cardinal offset
 * @param chunks Chunks array pointer
*/
void BRUT_FillChunks(t_chunks *chunks) {

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
u32 chunks_cube_get(t_chunks *chunks, vec3 *block_array, u32 chunkID)
{
    s8 next = TRUE;
	u32 idx = 0;

	(void)chunkID;

	for (u32 subID = 0; subID < SUBCHUNKS_DISPLAY; ++subID) {
		hashMap_it it = hashmap_iterator(chunks->sub_chunks[subID].block_map);
		next = hashmap_next(&it);
		while (next) {
			t_block *block = (t_block *)it.value;
			
			if (block->flag != BLOCK_HIDDEN) {
				block_array[idx][0] = (f32)block->x + (f32)(chunks->x * 16);
				block_array[idx][1] = (f32)block->y + (f32)(subID * SUB_CHUNKS_HEIGHT);
				block_array[idx][2] = (f32)block->z + (f32)(chunks->z * 16);
				if (idx == 0) {
					ft_printf_fd(1, "Block %u x:%f y:%f z:%f\n", idx, block_array[idx][0], block_array[idx][1], block_array[idx][2]);
				}
				++idx;
			}
			next = hashmap_next(&it);
		}
	}

    ft_printf_fd(1, GREEN"Renderer Cube %u\n"RESET, idx);
    return (idx);
}

/**
 * @brief Fill all chunks, call brut fill chunk on all chunkks and set hashmap
 * @param c Context pointer
*/
void fillChunks(hashMap *chunksMap) {
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


	for (u32 i = 0; i < TEST_CHUNK_MAX; i++) {
		t_chunks *chunks = ft_calloc(sizeof(t_chunks), 1);
		if (!chunks) {
			ft_printf_fd(2, "Failed to allocate chunks\n");
			return ;
		}

		chunks->id = i;
		chunks->x = off[chunks->id][0];
		chunks->z = off[chunks->id][1];
		ft_printf_fd(1, "Chunk %u x:%d z:%d\n", chunks->id, chunks->x, chunks->z);

		chunks->sub_chunks[0].block_map = hashmap_init(HASHMAP_SIZE_1000, hashmap_entry_free);
		// chunks->sub_chunks[1].block_map = hashmap_init(HASHMAP_SIZE_1000, hashmap_entry_free);
		BRUT_FillChunks(chunks);

		hashmap_set_entry(chunksMap, (t_block_pos){chunks->id, chunks->x, chunks->z}, chunks);
		// c->chunks[i].sub_chunks[0].block_map = hashmap_init(HASHMAP_SIZE_1000, hashmap_entry_free);
		// c->chunks[i].sub_chunks[1].block_map = hashmap_init(HASHMAP_SIZE_1000, hashmap_entry_free);
		// BRUT_FillChunks(&c->chunks[i]);
	}

}

/**
 * @brief Scan the environment around the player
 * @param c Context pointer
 * @param player_x Player's x position
 * @param player_z Player's z position
 * @param radius The radius around the player to scan
*/
// void scanEnvironment(t_context *c, s64 player_x, s64 player_z, u32 radius) {
//     // Calculate the chunk coordinates of the player
//     s64 player_chunk_x = player_x / CHUNK_SIZE;
//     s64 player_chunk_z = player_z / CHUNK_SIZE;

//     // Loop over all chunks in the radius around the player
//     for (s64 x = player_chunk_x - radius; x <= player_chunk_x + radius; x++) {
//         for (s64 z = player_chunk_z - radius; z <= player_chunk_z + radius; z++) {
//             // Calculate the ID of the chunk
//             u32 chunk_id = x + z * CHUNKS_PER_ROW;

//             // Check if the chunk is within the array bounds
//             if (chunk_id < TEST_CHUNK_MAX) {
//                 t_chunks *chunk = &c->chunks[chunk_id];

//                 // Loop over all blocks in the chunk
//                 for (u32 subID = 0; subID < SUBCHUNKS_DISPLAY; ++subID) {
//                     hashMap_it it = hashmap_iterator(chunk->sub_chunks[subID].block_map);
//                     s8 next = hashmap_next(&it);
//                     while (next) {
//                         t_block *block = (t_block *)it.value;

//                         // Perform an action with the block
//                         // ...

//                         next = hashmap_next(&it);
//                     }
//                 }
//             }
//         }
//     }
// }