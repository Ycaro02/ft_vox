#include "../../include/vox.h"

/**
 * @brief Check if all neighbors of a block exist
 * @param block_map the hashmap containing all blocks
 * @param x,y,z the position of the block
 * @return TRUE if all neighbors exist, FALSE otherwise
*/
s8 allNeighborsExist(hashMap *block_map, u32 x, u32 y, u32 z)
{
	if (hashmap_get(block_map, (t_block_pos){x + 1, y, z}) &&
		hashmap_get(block_map, (t_block_pos){x - 1, y, z}) &&
		hashmap_get(block_map, (t_block_pos){x, y + 1, z}) &&
		hashmap_get(block_map, (t_block_pos){x, y - 1, z}) &&
		hashmap_get(block_map, (t_block_pos){x, y, z + 1}) &&
		hashmap_get(block_map, (t_block_pos){x, y, z - 1}))
	{
		return (TRUE);
	}
	return (FALSE);
}

/* Occlusion Culling Strategy */

/**
 * @brief Check if a block is hidden and note it as such
 * @param chunks the chunks containing all blocks
 * @return the number of visible block
*/
u32 checkHiddenBlock(t_chunks *chunks)
{
    s8 next = TRUE;
	hashMap *block_map = chunks->sub_chunks[0].block_map;
	hashMap_it it = hashmap_iterator(block_map);
	next = hashmap_next(&it);
	u32 nb_block = hashmap_size(block_map);

	while (next) {
		t_block *block = (t_block *)it.value;
		if (allNeighborsExist(block_map, block->x, block->y, block->z)) {
			block->flag = BLOCK_HIDDEN;
			--nb_block;
		}
		next = hashmap_next(&it);	
	}
	return (nb_block);
}
