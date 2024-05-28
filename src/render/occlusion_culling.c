#include "../../include/world.h"
#include "../../include/chunks.h"

// void flagSet(u8 *flags, u8 flag_val)
// {
//     if (flags)
//         *flags |= flag_val;
// }

// void flagUnset(u8 *flags, u8 flag_val)
// {
//     if (flags)
//         *flags &= ~flag_val;
// }


// s8 flagIsSet(u8 flags, u8 flag_val) {
//    return ((flags & flag_val) == flag_val);
// }

s8 allNeighborsExist(Block *block) {
	return (block->neighbors == BLOCK_HIDDEN);
}

void updateNeighbors(Block *block, Block *blockCache[16][16][16]) {
    BlockPos pos[6] = {
        {block->x + 1, block->y, block->z}, {block->x - 1, block->y, block->z},
        {block->x, block->y + 1, block->z}, {block->x, block->y - 1, block->z},
        {block->x, block->y, block->z + 1}, {block->x, block->y, block->z - 1}
    };
    // These masks are for updating the neighbors of the current block
    u8 neighbor_masks[6] = {
        NEIGHBOR_RIGHT, NEIGHBOR_LEFT,
        NEIGHBOR_TOP, NEIGHBOR_BOTTOM,
        NEIGHBOR_FRONT, NEIGHBOR_BACK
    };
    // These masks are for updating the current block
    u8 block_masks[6] = {
        NEIGHBOR_LEFT, NEIGHBOR_RIGHT,
        NEIGHBOR_BOTTOM, NEIGHBOR_TOP,
        NEIGHBOR_BACK, NEIGHBOR_FRONT
    };

    for (u32 i = 0; i < 6; ++i) {
        if (pos[i].x >= 0 && pos[i].x < 16 && pos[i].y >= 0 && pos[i].y < 16 && pos[i].z >= 0 && pos[i].z < 16) {
            Block *neighbor = blockCache[pos[i].x][pos[i].y][pos[i].z];
            if (neighbor != NULL) {
                neighbor->neighbors |= neighbor_masks[i];
                block->neighbors |= block_masks[i];
            }
        }
    }
}


u32 checkHiddenBlock(Chunks *chunks, u32 subChunksID) {
    s8 next = TRUE;
    HashMap *block_map = chunks->sub_chunks[subChunksID].block_map;
    HashMap_it it = hashmap_iterator(block_map);
    next = hashmap_next(&it);
    u32 nb_block = hashmap_size(block_map);

    while (next) {
        Block *block = (Block *)it.value;
        if (allNeighborsExist(block)) {
            --nb_block;
        }
        next = hashmap_next(&it);
    }
    return (nb_block);
}



/* Occlusion Culling Strategy */


/**
 * @brief Check if all neighbors of a block exist
 * @param block_map the hashmap containing all blocks
 * @param x,y,z the position of the block
 * @return TRUE if all neighbors exist, FALSE otherwise
*/
// s8 allNeighborsExist(HashMap *block_map, u32 x, u32 y, u32 z)
// {
// 	if (hashmap_get(block_map, (BlockPos){x + 1, y, z}) &&
// 		hashmap_get(block_map, (BlockPos){x - 1, y, z}) &&
// 		hashmap_get(block_map, (BlockPos){x, y + 1, z}) &&
// 		hashmap_get(block_map, (BlockPos){x, y - 1, z}) &&
// 		hashmap_get(block_map, (BlockPos){x, y, z + 1}) &&
// 		hashmap_get(block_map, (BlockPos){x, y, z - 1}))
// 	{
// 		return (TRUE);
// 	}
// 	return (FALSE);
// }

/**
 * @brief Check if a block is hidden and note it as such
 * @param chunks the chunks containing all blocks
 * @return the number of visible block
*/
// u32 checkHiddenBlock(Chunks *chunks, u32 subChunksID)
// {
//     s8 next = TRUE;
// 	HashMap *block_map = chunks->sub_chunks[subChunksID].block_map;
// 	HashMap_it it = hashmap_iterator(block_map);
// 	next = hashmap_next(&it);
// 	u32 nb_block = hashmap_size(block_map);

// 	while (next) {
// 		Block *block = (Block *)it.value;
// 		if (allNeighborsExist(block_map, block->x, block->y, block->z)) {
// 			block->flag = BLOCK_HIDDEN;
// 			--nb_block;
// 		}
// 		next = hashmap_next(&it);	
// 	}
// 	return (nb_block);
// }