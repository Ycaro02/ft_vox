#ifndef HEADER_BIOME_H
#define HEADER_BIOME_H

#include "typedef_struct.h"

#define BIOME_PLAIN 0
#define BIOME_SNOW 1
#define BIOME_DESERT 2
#define BIOME_JUNGLE 3
#define BIOME_SWAMP 4

struct s_biome_block {
	s32		biomeId;
	s32		dirt;			/* Dirt for plain Biom */
	s32		top;			/* Grass top for plau Biom*/
	s32		water;			/* Water for plain biom */
	s32		underWater;		/* Underwater sand for plain biom */
	s32		stone;			/* Stone for plain biom */
};

/**
 * @brief biomeDetection, detect the biome of the block
 * @param biomBlock BiomBlock pointer (output)
 * @param dataNoise PerlinData, contain temperature and humidity for this block
*/
void biomDetection(BiomBlock *biomBlock, PerlinData dataNoise);
void treeCreate(Block ****subChunkBlockCache, SubChunks *subChunk, int x, int y, int z);

/*
	|-------------------------------------------------------------------------------------------|
	| Temperature \ Humidity | -1 to -0.6 | -0.6 to -0.2 | -0.2 to 0.2 | 0.2 to 0.6 | 0.6 to 1  |
	|------------------------|------------|--------------|-------------|------------|-----------|
	| -1 to -0.6             | Snow       | Snow         | Snow        | Plains     | Plains    |
	| -0.6 to -0.2           | Snow       | Snow         | Plains      | Plains     | Swamp     |
	| -0.2 to 0.2            | Snow       | Plains       | Plains      | Swamp      | Swamp     |
	| 0.2 to 0.6             | Plains     | Plains       | Swamp       | Jungle     | Jungle    |
	| 0.6 to 1               | Desert     | Desert       | Jungle      | Jungle     | Jungle    |
	|-------------------------------------------------------------------------------------------|
	-------------------------------------------------------------
	- Plain biom: ID 0
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


#endif /* HEADER_BIOME_H */
