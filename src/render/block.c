#include "../../include/camera.h"
#include "../../include/vox.h"
#include "../../include/world.h"
#include "../../include/render_chunks.h"
#include "../../rsc/perlin_noise/include/perlin_noise.h"
#include "../../include/chunks.h"
#include "../../include/block.h"
#include "../../include/biome.h"

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

/* Set local X and Z coordinates based on the center of the Perlin noise array */
s32 blockLocalToPerlinPos(s32 chunkOffset, s32 localPos, s32 width) {
	return ((chunkOffset * BLOCKS_PER_CHUNK + localPos) + (width / 2));
}

Block *basicBlockCreate(s32 x, s32 y, s32 z, s32 type) {
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
	block->isUnderground = FALSE;
	return (block);
}

Block *blockCreate(s32 x, s32 y, s32 z, s32 maxHeight, s32 startYWorld, BiomBlock *biomeBlock) {
    Block   	*block = NULL;
    s32     	blockType = AIR;
    s32     	realY = startYWorld + y;
	u8			isUnderground = 0;



	if (realY < maxHeight - 2) {
		blockType = biomeBlock->stone;
		isUnderground = TRUE;
		if (realY == 0) { blockType = BEDROCK; }
	} 
	else if (realY <= maxHeight) {
		blockType = biomeBlock->dirt;
		if (realY == maxHeight) { blockType = biomeBlock->top;}
		if (realY < SEA_LEVEL) { blockType = biomeBlock->underWater; }
	} 
	else if (realY == SEA_LEVEL) {
		blockType = biomeBlock->water;
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
	block->isUnderground = isUnderground;
	block->biomeId = biomeBlock->biomeId;
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
void underGroundBlockDataFill(s32 columnMaxHeight, vec3 *faceArray, s32 *faceTypeID, vec3 camPos, u8 face) {
	BlockPos	increment = {-1.0f, -1.0f, -1.0f};
	s32			blockType = STONE;
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
			/* Don't need biomeId here */
			faceTypeID[idx] = s32StoreValues(blockType, face, 0, 0);
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


	c->world->undergroundBlock->isUnderground = undergroundBoolUpdate(c, &currentBloc, &columnMaxHeight);
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
		underGroundBlockDataFill(columnMaxHeight, udg->udgFaceArray[i], udg->udgTypeID[i], camPos, i);
		udg->udgFaceVBO[i] = faceInstanceVBOCreate(udg->udgFaceArray[i], TOTAL_UNDERGROUND_FACE);
		udg->udgTypeVBO[i] = bufferGlCreate(GL_ARRAY_BUFFER, TOTAL_UNDERGROUND_FACE * sizeof(GLuint), (void *)udg->udgTypeID[i]);
		udg->udgFaceCount += TOTAL_UNDERGROUND_FACE;
	}
	ft_memcpy(&lastBlockPos, &currentBloc, sizeof(BlockPos));
}

#define CAM_INCAVE 2

/**
 * @brief Update the underground block boolean
 * @param c The game context
 * @param blockPos The block position to set [out]
*/
u8 undergroundBoolUpdate(Context *c, BlockPos *localBlockPos, s32 *columnMaxHeight) {
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
		return (FALSE);
	}
	*columnMaxHeight = chunk->noiseData[localBlockPos->x][localBlockPos->z].normalise;
	

	camPos[1] -= 0.333333f;
	if (camPos[1] < 0) {
		return (FALSE);
		// c->world->undergroundBlock->isUnderground = FALSE;
		// return ;
	}
	currentCamY = (s32)floor(camPos[1] *  2);
	
	block = worldPosProtectBlockGet(chunk, *localBlockPos, currentCamY);
	if (*columnMaxHeight <= currentCamY) {
		return (FALSE);
		// c->world->undergroundBlock->isUnderground = FALSE;
	}  else if (!block) {
		return (CAM_INCAVE);
		// c->world->undergroundBlock->isUnderground = FALSE;
	} 
	else {
		return (TRUE);
		// c->world->undergroundBlock->isUnderground = TRUE;
	}
}


// void blockWorldSpaceGet(Chunks *chunk, Block *block, s32 subChunkId, vec3 blockPos) {
// 	f32 x = ((f32)block->x * 0.5f) + (8.0f * (f32)chunk->x);
// 	f32 y = ((f32)block->y * 0.5f) + (f32)subChunkId * 8.0f;
// 	f32 z = ((f32)block->z * 0.5f) + (8.0f * (f32)chunk->z);
// 	// if (block->x == 0 && block->y == 0) {
// 	// 	ft_printf_fd(1, "Sub [%d] Block at [%d][%d][%d] (%f, %f, %f)\n", subChunkId, block->x, block->y, block->z ,x, y, z);
// 	// }
// 	blockPos[0] = x;
// 	blockPos[1] = y;
// 	blockPos[2] = z;
// }


