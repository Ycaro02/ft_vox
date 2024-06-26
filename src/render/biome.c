#include "../../include/biome.h"
#include "../../rsc/perlin_noise/include/perlin_noise.h"
#include "../../include/vox.h"
#include "../../include/block.h"
#include "../../include/chunks.h"

s32 blockBiomeIdGet(float temperature, float humidity) {
    static s32 biomeMap[5][5] = BIOME_ARRAY_INIT;

    s32 tempIndex = biomeMapIndexGet(temperature);
    s32 humidIndex = biomeMapIndexGet(humidity);
    
    return biomeMap[tempIndex][humidIndex];
}


void biomDetection(BiomBlock *biomBlock, s8 biomeId) {
	
	for (s32 i = 0; i < TREE_IDX_MAX; i++) {
		biomBlock->tree[i] = -1; /* No tree */
	}
	for (s32 i = 0; i < FLOWER_IDX_MAX; i++) {
		biomBlock->flowers[i] = -1; /* No flower */
	}

	if (biomeId == BIOME_SNOW) { /* Snow BIOM */
		biomBlock->top = SNOW_GRASS;
		biomBlock->dirt = DIRT;
		biomBlock->water = ICE;
		biomBlock->underWater = SNOW;
		biomBlock->stone = STONE;
	


		/* Trees */
		biomBlock->treeMax = 1;
		biomBlock->flowersMax = 3;
		// biomBlock->tree[0] = TREE_SPRUCE_LOG;
		biomBlock->tree[0] = 0;

		/* Flowers/Plants */
		biomBlock->flowers[0] = FLOWER_WHITE_TULIP;
		biomBlock->flowers[1] = FLOWER_DAYSIE;
		biomBlock->flowers[2] = PLANT_GRASS;
		
		return;
	}
	if (biomeId == BIOME_DESERT) { /* Desert BIOM */
		biomBlock->top = SANDSTONE;
		biomBlock->dirt = SANDSTONE;
		biomBlock->water = WATER;
		biomBlock->underWater = SAND;
		biomBlock->stone = SANDSTONE;
		
		biomBlock->treeMax = 2;
		biomBlock->flowersMax = 3;
		/* Trees */
		// biomBlock->tree[0] = TREE_ACACIA_LOG;
		// biomBlock->tree[1] = TREE_MANGROVE_LOG;
		biomBlock->tree[0] = 6;
		biomBlock->tree[1] = 2;

		/* Flowers/Plants */
		biomBlock->flowers[0] = PLANT_GRASS;
		biomBlock->flowers[1] = MUSHROOM_RED;
		biomBlock->flowers[2] = MUSHROOM_BROWN;

		
		return;
	}
	if (biomeId == BIOME_JUNGLE) { /* Jungle BIOM */
		biomBlock->top = GRASS;
		biomBlock->dirt = DIRT;
		biomBlock->water = WATER;
		biomBlock->underWater = SAND;
		biomBlock->stone = STONE;

		biomBlock->treeMax = 2;
		biomBlock->flowersMax = 6;
		/* Trees */
		// biomBlock->tree[0] = TREE_JUNGLE_LOG;
		// biomBlock->tree[1] = TREE_OAK_LOG;
		biomBlock->tree[0] = 3;
		biomBlock->tree[1] = 1;


		/* Flowers/Plants */
		biomBlock->flowers[0] = FLOWER_BLUE_ORCHID;
		biomBlock->flowers[1] = FLOWER_PINK_TULIP;
		biomBlock->flowers[2] = PLANT_FERN;
		biomBlock->flowers[3] = PLANT_GRASS;
		biomBlock->flowers[4] = MUSHROOM_RED;
		biomBlock->flowers[5] = MUSHROOM_BROWN;
		
		return;
	}
	if (biomeId == BIOME_SWAMP) { /* Swamp BIOM */
		biomBlock->top = GRASS;
		biomBlock->dirt = DIRT;
		biomBlock->water = WATER;
		biomBlock->underWater = SAND;
		biomBlock->stone = STONE;

		biomBlock->treeMax = 2;
		biomBlock->flowersMax = 6;
		/* Trees */
		// biomBlock->tree[0] = TREE_DARK_OAK_LOG;
		// biomBlock->tree[1] = TREE_OAK_LOG;
		biomBlock->tree[0] = 4;
		biomBlock->tree[1] = 1;


		/* Flowers/Plants */
		biomBlock->flowers[0] = FLOWER_BLUE_ORCHID;
		biomBlock->flowers[1] = FLOWER_LILY;
		biomBlock->flowers[2] = PLANT_FERN;
		biomBlock->flowers[3] = PLANT_GRASS;
		biomBlock->flowers[4] = MUSHROOM_RED;
		biomBlock->flowers[5] = MUSHROOM_BROWN;
		
		return;
	}
	/* Plain BIOM */
	biomBlock->top = GRASS;
	biomBlock->dirt = DIRT;
	biomBlock->water = WATER;
	biomBlock->underWater = SAND;
	biomBlock->stone = STONE;

	biomBlock->treeMax = 3;
	biomBlock->flowersMax = 10;
	/* Trees */
	// biomBlock->tree[0] = TREE_OAK_LOG;
	// biomBlock->tree[1] = TREE_BIRCH_LOG;
	// biomBlock->tree[2] = TREE_CHERRY_LOG;
	biomBlock->tree[0] = 1;
	biomBlock->tree[1] = 5;
	biomBlock->tree[2] = 7;

	/* Flowers/Plants */
	biomBlock->flowers[0] = FLOWER_DANDELION;
	biomBlock->flowers[1] = FLOWER_AZURE;
	biomBlock->flowers[2] = FLOWER_ALLIUM;
	biomBlock->flowers[3] = FLOWER_CORNFLOWER;
	biomBlock->flowers[4] = FLOWER_WHITE_TULIP;
	biomBlock->flowers[5] = FLOWER_RED_TULIP;
	biomBlock->flowers[6] = FLOWER_PINK_TULIP;
	biomBlock->flowers[7] = FLOWER_DAYSIE;
	biomBlock->flowers[8] = FLOWER_POPPY;
	biomBlock->flowers[9] = PLANT_GRASS;
}


typedef struct s_tree_texture {
	int log;
	int leaf;
} TreeTexture;


void expandSubChunkCheck(Chunks *chunk, s32 subChunkId) {
	if (!chunk->sub_chunks[subChunkId].block_map) {
		chunk->sub_chunks[subChunkId].block_map = hashmap_init(HASHMAP_SIZE_4000, hashmap_entry_free);
	}
}

void treeLeafGeneration(Block *****chunkBlockCache, Chunks *chunk, int vertexX, int vertexY, int vertexZ, int leafTexture) {
    Block *block = NULL;
	s32 cubeLen = 3;
    s32 startX = vertexX - 1;
    s32 startY = vertexY - 1;
    s32 startZ = vertexZ - 1;
	s32 endX = startX + cubeLen;
	s32 endY = startY + cubeLen;
	s32 endZ = startZ + cubeLen;

    s32 subChunkId = startY / 16;
	
	s32 x, y, z;
	// ft_printf_fd(1, "Input data for treeLeafGeneration: vertexX: %d, vertexY: %d, vertexZ: %d\n"RESET, vertexX, vertexY, vertexZ);

	if (startX < 0 || startY < 0 || startZ < 0
		|| endX > BLOCKS_PER_CHUNK || endZ > BLOCKS_PER_CHUNK) {
		return ;
	}

    for (x = startX; x < endX; x++) {
        for (y = startY; y < endY; y++) {
			subChunkId = y / 16;
			expandSubChunkCheck(chunk, subChunkId);
            for (z = startZ; z < endZ; z++) {
				s32 localY = y % 16;
				if ((x == vertexX && y == vertexY && z == vertexZ) || (x == vertexX && y == startY && z == vertexZ)) {
					continue;
				}
				block = basicBlockCreate(x, localY, z, leafTexture);
				if (!block) { continue ; }
				hashmap_set_entry(chunk->sub_chunks[subChunkId].block_map, (BlockPos){x, localY, z}, block);
				chunkBlockCache[subChunkId][x][localY][z] = block;
            }
        }
    }
}

void treeCreate(Block *****chunkBlockCache, Chunks *chunk, BlockPos pos, s32 treeId) {
	static TreeTexture treeTexture[] = {
		{TREE_SPRUCE_LOG, TREE_SPRUCE_LEAF},
		{TREE_OAK_LOG, TREE_OAK_LEAF},
		{TREE_MANGROVE_LOG, TREE_MANGROVE_LEAF},
		{TREE_JUNGLE_LOG, TREE_JUNGLE_LEAF},
		{TREE_DARK_OAK_LOG, TREE_DARK_OAK_LEAF},
		{TREE_BIRCH_LOG, TREE_BIRCH_LEAF},
		{TREE_ACACIA_LOG, TREE_ACACIA_LEAF},
		{TREE_CHERRY_LOG, TREE_CHERRY_LEAF}
	};
	Block	*block = NULL;
	s32 	subChunkId = 0;
	s32 	treeHeight = 5;
	s32 	vertexX = pos.x;
	s32 	vertexY = pos.y + treeHeight;
	s32 	vertexZ = pos.z;

	/* Build tree log */
	for (s32 i = 0; i < treeHeight; i++) {
		s32 localY = pos.y % 16;
		subChunkId = pos.y / 16;
		expandSubChunkCheck(chunk, subChunkId);
		block = basicBlockCreate(pos.x, localY, pos.z, treeTexture[treeId].log);
		if (!block) { return ; }
		hashmap_set_entry(chunk->sub_chunks[subChunkId].block_map, (BlockPos){pos.x, localY, pos.z}, block);
		chunkBlockCache[subChunkId][pos.x][localY][pos.z] = block;
		pos.y += 1;
	}
	treeLeafGeneration(chunkBlockCache, chunk, vertexX, vertexY, vertexZ, treeTexture[treeId].leaf);

}
