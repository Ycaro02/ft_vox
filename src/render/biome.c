#include "../../include/biome.h"
#include "../../rsc/perlin_noise/include/perlin_noise.h"
#include "../../include/vox.h"

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
