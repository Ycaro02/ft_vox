#include "../../include/camera.h"
#include "../../include/vox.h"
#include "../../include/world.h"
#include "../../include/render_chunks.h"
#include "../../rsc/perlin_noise/include/perlin_noise.h"
#include "../../include/chunks.h"
#include "../../include/block.h"

Block *getBlockAt(Chunks *chunk, u32 x, u32 y, u32 z, u32 subChunkID) {
	return (hashmap_get(chunk->sub_chunks[subChunkID].block_map, (BlockPos){x, y, z}));
}

Block *worldPosProtectBlockGet(Chunks *chunk, BlockPos localPos, s32 camY) {
	Block 	*block = NULL;
	s32 	subChunkID = camY / 16;
	s32 	maxSubChunk = chunk->noiseData[localPos.x][localPos.z].normalise / 16;
	if (subChunkID > maxSubChunk) {
		return (NULL);
	}
	block = getBlockAt(chunk, localPos.x, localPos.y, localPos.z, subChunkID);
	return (block);
}
#define BIOME_PLAIN 0
#define BIOME_SNOW 1
#define BIOME_DESERT 2
#define BIOME_JUNGLE 3
#define BIOME_SWAMP 4

typedef struct s_biom_block {
	s32		biomeId;
	s32		dirt;			/* Dirt for plain Biom */
	s32		top;			/* Grass top for plau Biom*/
	s32		water;			/* Water for plain biom */
	s32		underWater;		/* Underwater sand for plain biom */
	s32		stone;			/* Stone for plain biom */
} BiomBlock;

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

/* Set local X and Z coordinates based on the center of the Perlin noise array */
s32 blockLocalToPerlinPos(s32 chunkOffset, s32 localPos, s32 width) {
	return ((chunkOffset * BLOCKS_PER_CHUNK + localPos) + (width / 2));
}

Block *blockCreate(PerlinData **dataNoise, s32 x, s32 y, s32 z, s32 maxHeight, s32 startYWorld) {
    Block   	*block = NULL;
	BiomBlock	biomBlock = {0};
    s32     	blockType = AIR;
    s32     	realY = startYWorld + y;

	biomDetection(&biomBlock, dataNoise[x][z]); /* need perlin val here */

	if (realY < maxHeight - 2) {
		blockType = biomBlock.stone;
		if (realY == 0) { blockType = BEDROCK; }
	} 
	else if (realY <= maxHeight) {
		blockType = biomBlock.dirt;
		if (realY == maxHeight) { blockType = biomBlock.top;}
		if (realY < SEA_LEVEL) { blockType = biomBlock.underWater; }
	} 
	else if (realY == SEA_LEVEL) {
		blockType = biomBlock.water;
	} 
	else {
		return (NULL);
	}

    if (!(block = malloc(sizeof(Block)))) {
        ft_printf_fd(2, "Failed to allocate block\n");
        return (NULL);
    }
    block->type = blockType;
    block->x = x;
    block->y = y;
    block->z = z;
    block->neighbors = 0;
    return (block);
}


static void blockNegPosHandle(BlockPos *blockPos) {
    /* Handle neg for x and z */
    if (blockPos->x < 0) {
        blockPos->x = 16 + (blockPos->x % 16);
        if (blockPos->x == 16) blockPos->x = 0;
    } else {
        blockPos->x = blockPos->x % 16;
    }

    if (blockPos->z < 0) {
        blockPos->z = 16 + (blockPos->z % 16);
        if (blockPos->z == 16) blockPos->z = 0;
    } else {
        blockPos->z = blockPos->z % 16;
    }

    blockPos->y = blockPos->y % 16;
	if (blockPos->y < 0) { blockPos->y = 0; } 
}

void blockLocalPosFromCam(vec3 camPos, BlockPos *blockPos) {
 	blockPos->x = (s32)(floor(camPos[0] * 2.0f));
    blockPos->y = (s32)(floor(camPos[1] * 2.0f));
    blockPos->z = (s32)(floor(camPos[2] * 2.0f));
	blockNegPosHandle(blockPos);
}

/* Underground display logic */
/**
 * @brief Free the underground block data
 * @param udg The underground block data
*/
void undergroundBlockFree(UndergroundBlock *udg) {
	for (u8 i = 0; i < 6; ++i) {
		if (udg->udgFaceArray[i]) {
			free(udg->udgFaceArray[i]);
		}
		if (udg->udgTypeID[i]) {
			free(udg->udgTypeID[i]);
		}
		glDeleteBuffers(1, &udg->udgFaceVBO[i]);
		glDeleteBuffers(1, &udg->udgTypeVBO[i]);
	}
}

/**
 * @brief Fill the underground block data
 * @param faceArray The face array
 * @param faceTypeID The face type array
 * @param camPos The camera position
*/
void underGroundBlockDataFill(s32 columnMaxHeight, vec3 *faceArray, f32 *faceTypeID, vec3 camPos) {
	BlockPos	increment = {-1.0f, -1.0f, -1.0f};
	f32			blockType = STONE;
	s32 		idx = 0;
	s8 			firstIter = TRUE;

	if ((s32)(camPos[1] * 2.0f) >= columnMaxHeight - 1) {
		blockType = DIRT;
	}

	for (s32 layer = 0; layer < 3; layer++) {
		for (s32 i = 0; i < UNDERGROUND_FACE_NB; ++i) {
			if (!firstIter && i % 3 == 0) {
				increment.x += 1.0f;
				increment.z = -1.0f;
			}
			idx = i + (UNDERGROUND_FACE_NB * layer);
			faceArray[idx][0] = (camPos[0] * 2.0f) + increment.x;
			faceArray[idx][1] = (camPos[1] * 2.0f) + increment.y;
			faceArray[idx][2] = (camPos[2] * 2.0f) + increment.z;
			faceTypeID[idx] = blockType;
			increment.z += 1.0f;
			firstIter = FALSE;
		}
		increment.y += 1.0f;
		increment.x = -1.0f;
		increment.z = -1.0f;
		firstIter = TRUE;
	}

}


/**
 * @brief Create the underground block data for opengl context
 * @param c The game context
*/
void undergroundBlockcreate(Context *c) {
	static BlockPos 	lastBlockPos = {-1, -1, -1};
	vec3				camPos = {0};
	UndergroundBlock	*udg = NULL;
	BlockPos			currentBloc = {0};
	s32					columnMaxHeight = 0;


	undergroundBoolUpdate(c, &currentBloc, &columnMaxHeight);
	if (!c->world->undergroundBlock->isUnderground
		|| BLOCKPOS_CMP(lastBlockPos, currentBloc)) {
		return ;
	}

	mtx_lock(&c->gameMtx);
	glm_vec3_copy(c->cam->position, camPos);
	mtx_unlock(&c->gameMtx);

	if (c->world->undergroundBlock->udgFaceCount != 0) {
		undergroundBlockFree(c->world->undergroundBlock);
		c->world->undergroundBlock->udgFaceCount = 0;
	}

	
	udg = c->world->undergroundBlock;

	for (u8 i = 0; i < 6; ++i) {
		udg->udgFaceArray[i] = ft_calloc(sizeof(vec3), TOTAL_UNDERGROUND_FACE);
		udg->udgTypeID[i] = ft_calloc(sizeof(vec3), TOTAL_UNDERGROUND_FACE);
		underGroundBlockDataFill(columnMaxHeight, udg->udgFaceArray[i], udg->udgTypeID[i], camPos);
		udg->udgFaceVBO[i] = faceInstanceVBOCreate(udg->udgFaceArray[i], TOTAL_UNDERGROUND_FACE);
		udg->udgTypeVBO[i] = bufferGlCreate(GL_ARRAY_BUFFER, TOTAL_UNDERGROUND_FACE * sizeof(GLuint), (void *)udg->udgTypeID[i]);
		udg->udgFaceCount += TOTAL_UNDERGROUND_FACE;
	}
	ft_memcpy(&lastBlockPos, &currentBloc, sizeof(BlockPos));
}

/**
 * @brief Update the underground block boolean
 * @param c The game context
 * @param blockPos The block position to set [out]
*/
void undergroundBoolUpdate(Context *c, BlockPos *localBlockPos, s32 *columnMaxHeight) {
	vec3		camPos = {0.0f};
	Chunks		*chunk = NULL;
	Block 		*block = NULL;
	s32			currentCamY = 0, chunkPosx = 0, chunkPosz = 0;
	
	mtx_lock(&c->gameMtx);
	glm_vec3_copy(c->cam->position, camPos);
	chunkPosx = c->cam->chunkPos[0];
	chunkPosz = c->cam->chunkPos[2];
	mtx_unlock(&c->gameMtx);

	blockLocalPosFromCam(camPos, localBlockPos);
	chunk = hashmap_get(c->world->chunksMap, CHUNKS_MAP_ID_GET(chunkPosx, chunkPosz));
	if (!chunk) {
		return ;
	}
	*columnMaxHeight = chunk->noiseData[localBlockPos->x][localBlockPos->z].normalise;
	

	camPos[1] -= 0.333333f;
	if (camPos[1] < 0) {
		c->world->undergroundBlock->isUnderground = FALSE;
		return ;
	}
	currentCamY = (s32)floor(camPos[1] *  2);
	
	block = worldPosProtectBlockGet(chunk, *localBlockPos, currentCamY);
	if (!block) {
		c->world->undergroundBlock->isUnderground = FALSE;
	} else if (*columnMaxHeight <= currentCamY) {
		c->world->undergroundBlock->isUnderground = FALSE;
	} else {
		c->world->undergroundBlock->isUnderground = TRUE;
	}
}