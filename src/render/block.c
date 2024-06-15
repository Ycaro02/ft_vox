#include "../../include/render_chunks.h"
#include "../../include/perlin_noise.h"
#include "../../include/chunks.h"
#include "../../include/block.h"

Block *getBlockAt(Chunks *chunk, u32 x, u32 y, u32 z, u32 subChunkID) {
	(void)y;
	// return (hashmap_get(chunk->sub_chunks[y / 16].block_map, (BlockPos){x, y % 16, z}));
	return (hashmap_get(chunk->sub_chunks[subChunkID].block_map, (BlockPos){x, y, z}));
}


Block *worldPosProtectBlockGet(Chunks *chunk, BlockPos localPos, s32 camY) {
	Block 	*block = NULL;
	s32 	subChunkID = camY / 16;
	s32 	maxSubChunk = chunk->perlinVal[localPos.x][localPos.z].normalise / 16;
	if (subChunkID > maxSubChunk) {
		return (NULL);
	}
	block = getBlockAt(chunk, localPos.x, localPos.y, localPos.z, subChunkID);
	return (block);
}

/* Set local X and Z coordinates based on the center of the Perlin noise array */
s32 blockLocalToPerlinPos(s32 chunkOffset, s32 localPos, s32 width) {
	return ((chunkOffset * CHUNKS_NB_BLOCK + localPos) + (width / 2));
}

Block *blockCreate(s32 x, s32 y, s32 z, s32 maxHeight, s32 startYWorld) {
    Block   *block = NULL;
    s32     blockType = AIR;
    s32     realY = startYWorld + y;

	if (realY < maxHeight - 2) {
		blockType = STONE;
	} 
	else if (realY <= maxHeight) {
		blockType = DIRT;
		if ((realY == maxHeight || realY == maxHeight - 1) && realY >= SEA_LEVEL) { blockType = GRASS;}
	} 
	else if (realY == SEA_LEVEL) {
		blockType = WATER;
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

void blockPosFromCam(vec3 camPos, BlockPos *blockPos) {
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
void underGroundBlockDataFill(vec3 *faceArray, f32 *faceTypeID, vec3 camPos) {

	s32 incrementX = -1.0f;
	s32 incrementY = -1.0f;
	s32 incrementZ = -1.0f;
	s32 idx = 0;
	s8 	firstIter = TRUE;


	for (s32 layer = 0; layer < 3; layer++) {
		for (s32 i = 0; i < UNDERGROUND_FACE_NB; ++i) {
			if (!firstIter && i % 3 == 0) {
				incrementX += 1.0f;
				incrementZ = -1.0f;
			}
			idx = i + (UNDERGROUND_FACE_NB * layer);
			faceArray[idx][0] = (camPos[0] * 2.0f) + incrementX;
			faceArray[idx][1] = (camPos[1] * 2.0f) + incrementY;
			faceArray[idx][2] = (camPos[2] * 2.0f) + incrementZ;
			faceTypeID[idx] = (f32)STONE;
			incrementZ += 1.0f;
			firstIter = FALSE;
		}
		incrementY += 1.0f;
		incrementX = -1.0f;
		incrementZ = -1.0f;
		firstIter = TRUE;
	}

}


/**
 * @brief Create the underground block data for opengl context
 * @param c The game context
*/
void undergroundBlockcreate(Context *c) {
	vec3				camPos = {0};
	UndergroundBlock	*udg = NULL;
	BlockPos			currentBloc = {0};
	static BlockPos 	lastBlockPos = {-1, -1, -1};


	undergroundBoolUpdate(c, &currentBloc);
	if (!c->world->undergroundBlock->isUnderground
		|| BLOCKPOS_CMP(lastBlockPos, currentBloc)) {
		return ;
	}

	mtx_lock(&c->gameMtx);
	glm_vec3_copy(c->cam.position, camPos);
	mtx_unlock(&c->gameMtx);

	if (c->world->undergroundBlock->udgFaceCount != 0) {
		undergroundBlockFree(c->world->undergroundBlock);
		c->world->undergroundBlock->udgFaceCount = 0;
	}

	
	udg = c->world->undergroundBlock;

	for (u8 i = 0; i < 6; ++i) {
		udg->udgFaceArray[i] = ft_calloc(sizeof(vec3), TOTAL_UNDERGROUND_FACE);
		udg->udgTypeID[i] = ft_calloc(sizeof(vec3), TOTAL_UNDERGROUND_FACE);
		underGroundBlockDataFill(udg->udgFaceArray[i], udg->udgTypeID[i], camPos);
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
void undergroundBoolUpdate(Context *c, BlockPos *localBlockPos) {
	vec3		camPos = {0.0f};
	Chunks		*chunk = NULL;
	Block 		*block = NULL;
	s32			currentMaxHeight = 0, currentCamY = 0, chunkPosx = 0, chunkPosz = 0;
	
	mtx_lock(&c->gameMtx);
	glm_vec3_copy(c->cam.position, camPos);
	chunkPosx = c->cam.chunkPos[0];
	chunkPosz = c->cam.chunkPos[2];
	mtx_unlock(&c->gameMtx);

	blockPosFromCam(camPos, localBlockPos);
	chunk = hashmap_get(c->world->chunksMap, CHUNKS_MAP_ID_GET(chunkPosx, chunkPosz));
	if (!chunk) {
		return ;
	}
	currentMaxHeight = chunk->perlinVal[localBlockPos->x][localBlockPos->z].normalise;
	
	camPos[1] -= 0.2;
	currentCamY = (s32)floor(camPos[1] *  2);
	
	block = worldPosProtectBlockGet(chunk, *localBlockPos, currentCamY);
	if (!block) {
		c->world->undergroundBlock->isUnderground = FALSE;
	} else if (currentMaxHeight <= currentCamY) {
		c->world->undergroundBlock->isUnderground = FALSE;
	} else {
		c->world->undergroundBlock->isUnderground = TRUE;
	}
}