#include "../../include/vox.h"			/* Main project header */
#include "../../include/world.h"
#include "../../include/chunks.h"
#include "../../rsc/perlin_noise/include/perlin_noise.h"
#include "../../include/render_chunks.h"
#include "../../include/thread_load.h"
#include "../../include/block.h"
#include "../../include/cube.h"
#include "../../include/camera.h"

#define TRANSPARENT_BLOCK_MAX 11

s8 isTransparentBlock(s32 type) {
	const static s32 transparentBlock[] = {
		WATER, ICE, GLASS, TREE_SPRUCE_LEAF, TREE_OAK_LEAF
		, TREE_MANGROVE_LEAF, TREE_JUNGLE_LEAF, TREE_DARK_OAK_LEAF
		, TREE_BIRCH_LEAF, TREE_ACACIA_LEAF, TREE_CHERRY_LEAF};

	// s32 len = sizeof(transparentBlock) / sizeof(s32);

	for (s32 i = 0; i < TRANSPARENT_BLOCK_MAX; ++i) {
		if (type == transparentBlock[i]) {
			return (TRUE);
		}
	}
	return (blockIsFlowerPlants(type));
	// return (FALSE);
}

s8 isWaterIce(s32 type) {
	return (type == WATER || type == ICE);
}

s8 isTransparentNotWaterIce(s32 type) {
	return (isTransparentBlock(type) && !isWaterIce(type));
}

s8 faceHidden(u8 neighbors, u8 face) {
	return (neighbors & (1U << face));
}

u32 *faceVisibleCount(Chunks *chunks, u32 *transparentFaceCount, u32 *waterFaceCount) {
	u32 *opaqueCount = ft_calloc(sizeof(u32), 6);

	for (s32 subID = 0; chunks->sub_chunks[subID].block_map != NULL; ++subID) {
		HashMap_it it = hashmap_iterator(chunks->sub_chunks[subID].block_map);
		while (hashmap_next(&it)) {
			Block *block = (Block *)it.value;
			for (u8 i = 0; i < 6; ++i) {
				if (!faceHidden(block->neighbors, i)) {
					if (!isTransparentBlock(block->type)) {
						opaqueCount[i] += 1U;
					} else if (isTransparentNotWaterIce(block->type)) {
						transparentFaceCount[i] += 1U;
					} else if (isWaterIce(block->type) && i == TOP_FACE) {
						*waterFaceCount += 1U;
					}
				}
			}
		}
	}
	return (opaqueCount);
}

void displayAllAtlasBlock(f32 x, f32 z, s32 *type, u8 face) {
	// (void)x, (void)z;
	int textureIdx = x + z * 16;
	if (textureIdx <= NEWAT_MAX) {
		*type = s32StoreValues(textureIdx, face, 0, 0);
	} 
}

void woolDebugFog(s32 chunkX,s32 chunkZ, s32 *type) {
	static s8 woolArray[] = {
		NEWAT_RED_WOOL,
		NEWAT_ORANGE_WOOL,
		NEWAT_YELLOW_WOOL,
		NEWAT_LIME_WOOL,
		NEWAT_GREEN_WOOL,
		NEWAT_CYAN_WOOL,
		NEWAT_LIGHT_BLUE_WOOL,
		NEWAT_BLUE_WOOL,
		NEWAT_PURPLE_WOOL,
		NEWAT_MAGENTA_WOOL,
		NEWAT_PINK_WOOL,
		NEWAT_BLACK_WOOL,
		NEWAT_BROWN_WOOL
	};
	s32 len = sizeof(woolArray) / sizeof(s8);

	if (chunkZ == 0) {
		*type = s32StoreValues(woolArray[abs(chunkX) % len], BOTTOM_FACE, 0, 0);
	}
}

/* To call in create render chunk -> DONE */
void chunksCubeFaceGet(Mutex *chunkMtx, Chunks *chunks, RenderChunks *render)
{
	(void)chunkMtx;

	u32 opqIdx[6] = {0};
	u32 trspIdx[6] = {0};
	u32 waterFaceIdx = 0;
	u8  lastDataByte = 0;

	ft_bzero(opqIdx, sizeof(u32) * 6);
	ft_bzero(trspIdx, sizeof(u32) * 6);
	render->topFaceWaterCount = 0;
	render->trspFaceCount = ft_calloc(sizeof(u32), 6);
	render->faceCount = faceVisibleCount(chunks, render->trspFaceCount, &render->topFaceWaterCount);


	for (u8 i = 0; i < 6; ++i) {
		/* Opaque face init */
		render->faceArray[i] = ft_calloc(sizeof(vec3), render->faceCount[i]);
		render->faceTypeID[i] = ft_calloc(sizeof(f32), render->faceCount[i]);
		/* Trsp face init */
		render->trspFaceArray[i] = ft_calloc(sizeof(vec3), render->trspFaceCount[i]);
		render->trspTypeId[i] = ft_calloc(sizeof(f32), render->trspFaceCount[i]);
	}
	render->topFaceWater = ft_calloc(sizeof(vec3), render->topFaceWaterCount);
	render->topFaceWaterTypeID = ft_calloc(sizeof(f32), render->topFaceWaterCount);

	for (s32 subID = 0; chunks->sub_chunks[subID].block_map != NULL; ++subID) {
		HashMap_it it = hashmap_iterator(chunks->sub_chunks[subID].block_map);
		while (hashmap_next(&it)) {
			Block *block = (Block *)it.value;
			for (u8 i = 0; i < 6; ++i) {
				if (!faceHidden(block->neighbors, i)) {
					if (!isTransparentBlock(block->type)) {
						render->faceArray[i][opqIdx[i]][0] = (f32)block->x + (f32)(chunks->x * 16);
						render->faceArray[i][opqIdx[i]][1] = (f32)block->y + (f32)(subID * 16);
						render->faceArray[i][opqIdx[i]][2] = (f32)block->z + (f32)(chunks->z * 16);
						lastDataByte = u8StoreValues(blockIsFlowerPlants(block->type),0,0,0,0,0,0,0);
						render->faceTypeID[i][opqIdx[i]] = s32StoreValues(block->type, i, chunks->biomeId, lastDataByte);
						if (chunks->x == 0 && chunks->z == 0 && render->faceArray[i][opqIdx[i]][1] == 0) {
							displayAllAtlasBlock(render->faceArray[i][opqIdx[i]][0], render->faceArray[i][opqIdx[i]][2], &render->faceTypeID[i][opqIdx[i]], i);
						}
						// woolDebugFog(chunks->x, chunks->z, &render->faceTypeID[i][opqIdx[i]]);
						opqIdx[i] += 1;
					} else if (isTransparentNotWaterIce(block->type)) { /* Water face fill */
						render->trspFaceArray[i][trspIdx[i]][0] = (f32)block->x + (f32)(chunks->x * 16);
						render->trspFaceArray[i][trspIdx[i]][1] = (f32)block->y + (f32)(subID * 16);
						render->trspFaceArray[i][trspIdx[i]][2] = (f32)block->z + (f32)(chunks->z * 16);
						lastDataByte = u8StoreValues(blockIsFlowerPlants(block->type),0,0,0,0,0,0,0);
						render->trspTypeId[i][trspIdx[i]] = s32StoreValues(block->type, i, chunks->biomeId, lastDataByte);
						trspIdx[i] += 1;
					} else if (isWaterIce(block->type) && i == TOP_FACE) { /* Water face fill */
						render->topFaceWater[waterFaceIdx][0] = (f32)block->x + (f32)(chunks->x * 16);
						render->topFaceWater[waterFaceIdx][1] = (f32)block->y + (f32)(subID * 16);
						render->topFaceWater[waterFaceIdx][2] = (f32)block->z + (f32)(chunks->z * 16);
						render->topFaceWaterTypeID[waterFaceIdx] = s32StoreValues(block->type, i, chunks->biomeId, 0);
						waterFaceIdx++;
					}

				}

			}
		}
	}

}


GLuint faceInstanceVBOCreate(vec3 *faceArray, u32 faceNb) {
    return (bufferGlCreate(GL_ARRAY_BUFFER, faceNb * sizeof(vec3), (void *)faceArray[0]));
}

void renderChunkCreateFaceVBO(HashMap *chunksMap, BlockPos chunkID) {
	/* Create VBO */
	Chunks 			*chunks = NULL; 
	RenderChunks 	*render = NULL; 

	chunks = hashmap_get(chunksMap, chunkID);
	if (!chunks || !chunks->render) {
		return ;
	}
	render = chunks->render;
	for (u8 i = 0; i < 6; ++i) {
		render->faceVBO[i] = faceInstanceVBOCreate(render->faceArray[i], render->faceCount[i]);
		render->faceTypeVBO[i] = bufferGlCreate(GL_ARRAY_BUFFER, render->faceCount[i] * sizeof(GLuint), (void *)&render->faceTypeID[i][0]);
	
		render->trspFaceVBO[i] = faceInstanceVBOCreate(render->trspFaceArray[i], render->trspFaceCount[i]);
		render->trspTypeVBO[i] = bufferGlCreate(GL_ARRAY_BUFFER, render->trspFaceCount[i] * sizeof(GLuint), (void *)&render->trspTypeId[i][0]);
	}

	render->topFaceWaterVBO = faceInstanceVBOCreate(render->topFaceWater, render->topFaceWaterCount);
	render->topFaceWaterTypeVBO = bufferGlCreate(GL_ARRAY_BUFFER, render->topFaceWaterCount * sizeof(GLuint), (void *)&render->topFaceWaterTypeID[0]);

}

void drawFace(GLuint faceVBO, GLuint metadataVBO, u32 vertex_nb, u32 faceNb) {
	/* Bind Block instance VBO */
	glBindBuffer(GL_ARRAY_BUFFER, faceVBO);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glVertexAttribDivisor(1, 1);

	/* Bind Metadata VBO */
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, metadataVBO);
	glEnableVertexAttribArray(3);
	/* Care here we need to use glVertexAttribIPointer */
	glVertexAttribIPointer(3, 1, GL_INT, sizeof(s32), (void*)0);
	glVertexAttribDivisor(3, 1);
	
	glDrawElementsInstanced(GL_TRIANGLES, vertex_nb, GL_UNSIGNED_INT, 0, faceNb);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


typedef struct s_render_chunk_cache {
	RenderChunks	*render;
	s32				distance;
} RenderChunkCache;


void renderChunksCacheFill(RenderChunks *render, RenderChunkCache *cache,  vec2_s32 cameraChunk, u32 *cacheIdx) {
	cache[*cacheIdx].render = render;
	cache[*cacheIdx].distance = chunksEuclideanDistanceGet(cameraChunk[0], cameraChunk[1], cache[*cacheIdx].render->chunkID.x, cache[*cacheIdx].render->chunkID.z);
	*cacheIdx += 1U;
}

void opaqueFaceDisplay(Context *c, RenderChunkCache *cache, vec2_s32 cameraChunk) {
	HashMap_it		it;
	RenderChunks	*render = NULL;
    u32 			faceNb = 0;
	GLuint 			faceVBO = 0, faceTypeVBO = 0;
	u32				cacheIdx = 0;

	for (u8 i = 0; i < 6; ++i) {
		glBindVertexArray(c->faceCube[i].VAO);
		it = hashmap_iterator(c->world->renderChunksMap);
		while (hashmap_next(&it)) {
			render = (RenderChunks *)it.value;
			faceNb = render->faceCount[i];
			faceVBO = render->faceVBO[i];
			faceTypeVBO = render->faceTypeVBO[i];
			c->displayData.faceRendered += faceNb;
			if (i == 0) {
				renderChunksCacheFill(render, cache, cameraChunk, &cacheIdx);
			}
			drawFace(faceVBO, faceTypeVBO, 6U, faceNb);
		}
		/* Underground face display */
		if (c->world->undergroundBlock->isUnderground == TRUE && c->displayUndergroundBlock) {
			drawFace(c->world->undergroundBlock->udgFaceVBO[i], c->world->undergroundBlock->udgTypeVBO[i], 6U, TOTAL_UNDERGROUND_FACE);			
		}
		glBindVertexArray(0);
	}
}

void renderWater(RenderChunkCache *cache, u32 *faceRenderer, GLuint VAO) {
	RenderChunks	*render = NULL;
	u32 			faceNb = 0;
	s32 			i = 0;

	/* Display water face before */
	glBindVertexArray(VAO);
	while (cache[i].render) {
		render = cache[i].render;
		faceNb = render->topFaceWaterCount;
		*faceRenderer += faceNb;
		drawFace(render->topFaceWaterVBO, render->topFaceWaterTypeVBO, 6U, faceNb);
		i++;
	}
	glBindVertexArray(0);
}

void trspFaceDisplay(Context *c, RenderChunkCache *cache) {
	
	RenderChunks	*render = NULL;
	u32 			faceNb = 0;
	GLuint 			faceVBO = 0, faceTypeVBO = 0;
	s32 			count = 0;
	s8				waterDisplayBefore = c->displayData.blockPos.y > SEA_LEVEL;


	if (waterDisplayBefore) {
		renderWater(cache, &c->displayData.faceRendered, c->faceCube[TOP_FACE].VAO);
	}

	for (s32 i = 5; i >= 0; --i) {
		count = 0;
		glBindVertexArray(c->faceCube[i].VAO);
		while (cache[count].render) {
			render = cache[count].render;
			faceNb = render->trspFaceCount[i];
			faceVBO = render->trspFaceVBO[i];
			faceTypeVBO = render->trspTypeVBO[i];
			c->displayData.faceRendered += faceNb;
			drawFace(faceVBO, faceTypeVBO, 6U, faceNb);
			count++;
		}
		glBindVertexArray(0);
	}

	if (!waterDisplayBefore) {
		renderWater(cache, &c->displayData.faceRendered, c->faceCube[TOP_FACE].VAO);
	}
}

void renderCacheSort(RenderChunkCache *array, s32 n) {
    RenderChunkCache	tmp;
    s32					i, j;
    
	for (i = 1; i < n; i++) {
        tmp = array[i];
        j = i - 1;
        while (j >= 0 && array[j].distance < tmp.distance) {
            array[j + 1] = array[j];
            j = j - 1;
        }
        array[j + 1] = tmp;
    }
}

/**
 * @brief Draw all chunks by face
 * @param *c Context
*/
void drawAllChunksByFace(Context *c) {
	RenderChunkCache 	*cache = NULL;
	vec2_s32			cameraChunk = {0, 0};

	mtx_lock(&c->gameMtx);
	cameraChunk[0] = c->cam->chunkPos[0]; /* x */
	cameraChunk[1] = c->cam->chunkPos[2]; /* z */
	mtx_unlock(&c->gameMtx);

    mtx_lock(&c->renderMtx);
	c->displayData.chunkRenderedNb = hashmap_size(c->world->renderChunksMap);
	c->displayData.faceRendered = 0;
	cache = malloc(sizeof(RenderChunkCache) * (c->displayData.chunkRenderedNb + 1));
	cache[c->displayData.chunkRenderedNb].render = NULL;
	cache[c->displayData.chunkRenderedNb].distance = -1;
	opaqueFaceDisplay(c, cache, cameraChunk);
	renderCacheSort(cache, c->displayData.chunkRenderedNb);
	trspFaceDisplay(c, cache);
	free(cache);
    mtx_unlock(&c->renderMtx);
}