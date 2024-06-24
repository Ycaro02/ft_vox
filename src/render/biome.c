#include "../../include/biome.h"
#include "../../rsc/perlin_noise/include/perlin_noise.h"
#include "../../include/vox.h"
#include "../../include/block.h"
#include "../../include/chunks.h"

s32 getBiomeMapIndex(float value) {
	if (value < -0.6) return 0;
	else if (value < -0.2) return 1;
	else if (value < 0.2) return 2;
	else if (value < 0.6) return 3;
	else return 4;
}

s32 getBiomeId(float temperature, float humidity) {
    static s32 biomeMap[5][5] = {
        {BIOME_SNOW,  BIOME_SNOW,  BIOME_SNOW,  BIOME_PLAIN,  BIOME_PLAIN},
        {BIOME_SNOW,  BIOME_SNOW,  BIOME_PLAIN, BIOME_PLAIN,  BIOME_SWAMP},
        {BIOME_SNOW,  BIOME_PLAIN, BIOME_PLAIN, BIOME_SWAMP,  BIOME_SWAMP},
        {BIOME_PLAIN, BIOME_PLAIN, BIOME_SWAMP, BIOME_JUNGLE, BIOME_JUNGLE},
        {BIOME_DESERT, BIOME_DESERT, BIOME_JUNGLE, BIOME_JUNGLE, BIOME_JUNGLE}
    };

    s32 tempIndex = getBiomeMapIndex(temperature);
    s32 humidIndex = getBiomeMapIndex(humidity);
    
    return biomeMap[tempIndex][humidIndex];
}


void biomDetection(BiomBlock *biomBlock, PerlinData dataNoise) {
	biomBlock->biomeId = getBiomeId(dataNoise.valTemperature, dataNoise.valHumidity);
	if (biomBlock->biomeId == BIOME_SNOW) { /* Snow BIOM */
		biomBlock->top = SNOW_GRASS;
		biomBlock->dirt = DIRT;
		biomBlock->water = ICE;
		biomBlock->underWater = SNOW;
		biomBlock->stone = STONE;
		return;
	}
	if (biomBlock->biomeId == BIOME_DESERT) { /* Desert BIOM */
		biomBlock->top = SANDSTONE;
		biomBlock->dirt = SANDSTONE;
		biomBlock->water = WATER;
		biomBlock->underWater = SAND;
		biomBlock->stone = SANDSTONE;
		return;
	}
	/**
	 * Need to implement jungle and swamp
	*/

	/* Plain BIOM */
	biomBlock->top = GRASS;
	biomBlock->dirt = DIRT;
	biomBlock->water = WATER;
	biomBlock->underWater = SAND;
	biomBlock->stone = STONE;

}

typedef struct s_tree_texture {
	int log;
	int leaf;
} TreeTexture;


Block *basicBlockCreate(int x, int y, int z, int type) {
	Block *block = malloc(sizeof(Block));
	if (!block) {
		ft_printf_fd(2, "Failed to allocate block\n");
		return (NULL);
	}
	block->x = x;
	block->y = y;
	block->z = z;
	block->neighbors = 0;
	block->biomeId = 0;
	block->type = type;
	return (block);
}

void treeLeafGeneration(Block ****subChunkBlockCache, SubChunks *subChunk, int vertexX, int vertexY, int vertexZ, int leafTexture) {
    Block *block = NULL;
	s32 cubeLen = 3;
    s32 startX = vertexX - 1;
    s32 startY = vertexY - 1;
    s32 startZ = vertexZ - 1;
    s32 x, y, z;
	// ft_printf_fd(1, "Input data for treeLeafGeneration: vertexX: %d, vertexY: %d, vertexZ: %d\n"RESET, vertexX, vertexY, vertexZ);

    for (x = startX; x < startX + cubeLen; x++) {
        for (y = startY; y < startY + cubeLen; y++) {
            for (z = startZ; z < startZ + cubeLen; z++) {
				s32 localY = y % 16;
				if ((x == vertexX && y == vertexY && z == vertexZ) || (x == vertexX && y == startY && z == vertexZ)) {
					continue;
				}
				block = basicBlockCreate(x, localY, z, leafTexture);
				hashmap_set_entry(subChunk->block_map, (BlockPos){x, localY, z}, block);
				subChunkBlockCache[x][localY][z] = block;
                // ft_printf_fd(1, GREEN"Leaf: (%d, %d, %d)\n"RESET, x, y, z);
            }
        }
    }
}



void treeCreate(Block ****subChunkBlockCache, SubChunks *subChunk, s32 x, s32 y, s32 z, s32 treeId) {
	static TreeTexture treeTexture[] = {
		{TREE_SPRUCE_LOG, TREE_SPRUCE_LEAF},
		{TREE_OAK_LOG, TREE_OAK_LEAF},
		{TREE_MANGROVE_LOG, TREE_MANGROVE_LEAF},
		{TREE_JUNGLE_LOG, TREE_JUNGLE_LEAF},
		{TREE_DARK_OAK_LOG, TREE_DARK_OAK_LEAF},
		{TREE_BIRCH_LOG, TREE_BIRCH_LEAF},
		{TREE_ACACIA_LOG, TREE_ACACIA_LEAF}
		};

	s32 treeHeight = 4;
	s32 vertexX = x;
	s32 vertexY = y + treeHeight;
	s32 vertexZ = z;

	/* Build tree log */
	for (s32 i = 0; i < treeHeight; i++) {
		s32 localY = y % 16;
		if (localY > BLOCKS_PER_CHUNK) {
			return ;
		}
		// ft_printf_fd(1, "Tree log: x: %d, y: %d, z: %d: LocalY %d\n", x, y, z, localY);
		Block *block = basicBlockCreate(x, localY, z, treeTexture[treeId].log);
		if (!block) { return ; }
		hashmap_set_entry(subChunk->block_map, (BlockPos){x, localY, z}, block);
		subChunkBlockCache[x][localY][z] = block;
		y++;
	}
	treeLeafGeneration(subChunkBlockCache, subChunk, vertexX, vertexY, vertexZ, treeTexture[treeId].leaf);

}
