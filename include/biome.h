#ifndef HEADER_BIOME_H
#define HEADER_BIOME_H

#include "typedef_struct.h"

#define BIOME_PLAIN 0
#define BIOME_SNOW 1
#define BIOME_DESERT 2
#define BIOME_JUNGLE 3
#define BIOME_SWAMP 4

#define TREE_IDX_MAX 8

struct s_biome_block {
	s32		biomeId;
	s32		dirt;				/* Dirt for plain Biom */
	s32		top;				/* Grass top for plau Biom*/
	s32		water;				/* Water for plain biom */
	s32		underWater;			/* Underwater sand for plain biom */
	s32		stone;				/* Stone for plain biom */
	s8		tree[TREE_IDX_MAX];	/* Tree texture */
};


/*
    Biome array rules:
    |-------------------------------------------------------------------------------------------|
    | Temperature \ Humidity | -1 to -0.4 | -0.4 to 0.0 | 0.0 to 0.4  | 0.4 to 0.7 | 0.7 to 1   |
    |------------------------|------------|-------------|-------------|------------|------------|
    | -1 to -0.4             | Snow       | Snow        | Snow        | Plains     | Plains     |
    | -0.4 to 0.0            | Snow       | Snow        | Plains      | Plains     | Swamp      |
    | 0.0 to 0.4             | Plains     | Plains      | Swamp       | Swamp      | Jungle     |
    | 0.4 to 0.7             | Plains     | Swamp       | Swamp       | Jungle     | Desert     |
    | 0.7 to 1               | Jungle     | Jungle      | Jungle      | Desert     | Desert     |
    |-------------------------------------------------------------------------------------------|
*/

#define BIOME_ARRAY_INIT {\
    {BIOME_SNOW,  BIOME_SNOW,  BIOME_SNOW,  BIOME_PLAIN,  BIOME_PLAIN},\
    {BIOME_SNOW,  BIOME_SNOW,  BIOME_PLAIN, BIOME_PLAIN,  BIOME_SWAMP},\
    {BIOME_PLAIN, BIOME_PLAIN, BIOME_SWAMP, BIOME_SWAMP,  BIOME_JUNGLE},\
    {BIOME_PLAIN, BIOME_SWAMP, BIOME_SWAMP, BIOME_JUNGLE, BIOME_DESERT},\
    {BIOME_JUNGLE, BIOME_JUNGLE, BIOME_JUNGLE, BIOME_DESERT, BIOME_DESERT}\
}

FT_INLINE s8 biomeMapIndexGet(float value) {
    if (value <= -0.4) { return 0; }
    else if (value <= 0.0) { return 1; }
    else if (value <= 0.4) { return 2; }
    else if (value <= 0.7) { return 3; }
    return 4;
}

s32 blockBiomeIdGet(float temperature, float humidity);
/**
 * @brief biomeDetection, detect the biome of the block
 * @param biomBlock BiomBlock pointer (output)
 * @param biomeId Biome ID
*/
void biomDetection(BiomBlock *biomBlock, s8 biomeId);


void treeCreate(Block *****chunkBlockCache, Chunks *Chunk, BlockPos pos, s32 treeId);
/*
	-------------------------------------------------------------
	- Plain biom: ID 0, Jungle biom: ID 3, Swamp biom: ID 4
		- Dirt: DIRT
		- Top: GRASS
		- Water: WATER
		- Underwater: SAND
		- Stone: STONE
	-------------------------------------------------------------
	- Snow biom: ID 1
		- Dirt: DIRT
		- Top: SNOW_GRASS
		- Water: ICE
		- Underwater: SNOW
		- Stone: STONE
	-------------------------------------------------------------
	- Desert biom: ID 2
		- Dirt: SANDSTONE
		- Top: SAND
		- Water: WATER
		- Underwater: SAND
		- Stone: STONE
 */

/*
	TO DO:
		- add tree selection depend of biome
		- add flower render and selection depend of biome
		- add mouse move camera on x and y axis
*/


#endif /* HEADER_BIOME_H */
