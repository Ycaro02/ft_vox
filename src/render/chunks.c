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
	t_cardinal_offset chunk_offset[] = {
		{0, 0, 0, 0},\
		{16, 0, 0, 0},\
		{0, 16, 0, 0},\
		{0, 0, 16, 0},\
		{0, 0, 0, 16},\
		{16, 0, 16, 0},\
		{16, 0, 0, 16},\
		{0, 16, 16, 0},\
		{0, 16, 0, 16},\
	};
	chunks->offset = chunk_offset[chunks->id]; 
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
	s64 x_offset = 0, z_offset = 0;
	x_offset = (s64)(chunks->offset.north) - (s64)(chunks->offset.south); 
	z_offset = (s64)(chunks->offset.west) - (s64)(chunks->offset.east);

	for (u32 subID = 0; subID < SUBCHUNKS_DISPLAY; ++subID) {
		hashMap_it it = hashmap_iterator(chunks->sub_chunks[subID].block_map);
		next = hashmap_next(&it);
		while (next) {
			t_block *block = (t_block *)it.value;
			
			if (block->flag != BLOCK_HIDDEN) {
				block_array[idx][0] = block->x + x_offset;
				block_array[idx][1] = block->y + (subID * SUB_CHUNKS_HEIGHT);
				block_array[idx][2] = block->z + z_offset;
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
void fillChunks(t_context *c) {
	for (u32 i = 0; i < TEST_CHUNK_MAX; i++) {
		c->chunks[i].sub_chunks[0].block_map = hashmap_init(HASHMAP_SIZE_1000, hashmap_entry_free);
		c->chunks[i].sub_chunks[1].block_map = hashmap_init(HASHMAP_SIZE_1000, hashmap_entry_free);
		c->chunks[i].id = i;
		BRUT_FillChunks(&c->chunks[i]);
	}

}