#include "../../include/vox.h"			/* Main project header */
#include "../../include/world.h"
#include "../../include/chunks.h"
#include "../../rsc/perlin_noise/include/perlin_noise.h"
#include "../../include/render_chunks.h"
#include "../../include/thread_load.h"
#include "../../include/block.h"
#include "../../include/cube.h"

s8 isTransparentBlock(s32 type) {
	const static s32 transparentBlock[] = {
		WATER, ICE, GLASS, TREE_SPRUCE_LEAF, TREE_OAK_LEAF
		, TREE_MANGROVE_LEAF, TREE_JUNGLE_LEAF, TREE_DARK_OAK_LEAF
		, TREE_BIRCH_LEAF, TREE_ACACIA_LEAF, TREE_CHERRY_LEAF};

	s32 len = sizeof(transparentBlock) / sizeof(s32);

	for (u8 i = 0; i < len; ++i) {
		if (type == transparentBlock[i]) {
			return (TRUE);
		}
	}
	return (FALSE);
}

s8 isWaterIce(s32 type) {
	return (type == WATER || type == ICE);
}

s8 faceHidden(u8 neighbors, u8 face) {
	return (neighbors & (1U << face));
}

u32 *faceVisibleCount(Chunks *chunks, u32 *transparentFaceCount) {
	u32 *opaqueCount = ft_calloc(sizeof(u32), 6);
    // s8 	next = TRUE;


	for (s32 subID = 0; chunks->sub_chunks[subID].block_map != NULL; ++subID) {
		HashMap_it it = hashmap_iterator(chunks->sub_chunks[subID].block_map);
		while (hashmap_next(&it)) {
			Block *block = (Block *)it.value;
			for (u8 i = 0; i < 6; ++i) {
				if (!faceHidden(block->neighbors, i) && !isTransparentBlock(block->type)) {
					opaqueCount[i] += 1U;
				}
				if (isTransparentBlock(block->type) && !faceHidden(block->neighbors, i)) {
					transparentFaceCount[i] += 1U;
				}
			}
		}
	}

	return (opaqueCount);
}

void displayAllAtlasBlock(f32 x, f32 z, s32 *type) {
	// (void)x, (void)z;
	int textureIdx = x + z * 16;
	if (textureIdx <= NEWAT_MAX) {
		*type = s32StoreValues(textureIdx, 0, 0, 0);
	} 
}

/* To call in create render chunk -> DONE */
void chunksCubeFaceGet(Mutex *chunkMtx, Chunks *chunks, RenderChunks *render)
{
	(void)chunkMtx;

	u32 opqIdx[6] = {0};
	u32 trspIdx[6] = {0};

	// ft_bzero(opqIdx, sizeof(u32) * 6);
	// ft_bzero(trspIdx, sizeof(u32) * 6);


	render->trspFaceCount = ft_calloc(sizeof(u32), 6);
	render->faceCount = faceVisibleCount(chunks, render->trspFaceCount);
	for (u8 i = 0; i < 6; ++i) {
		/* Opaque face init */
		render->faceArray[i] = ft_calloc(sizeof(vec3), render->faceCount[i]);
		render->faceTypeID[i] = ft_calloc(sizeof(f32), render->faceCount[i]);
		/* Trsp face init */
		render->trspFaceArray[i] = ft_calloc(sizeof(vec3), render->trspFaceCount[i]);
		render->trspTypeId[i] = ft_calloc(sizeof(f32), render->trspFaceCount[i]);
	}


	for (s32 subID = 0; chunks->sub_chunks[subID].block_map != NULL; ++subID) {
		HashMap_it it = hashmap_iterator(chunks->sub_chunks[subID].block_map);
		while (hashmap_next(&it)) {
			Block *block = (Block *)it.value;
			for (u8 i = 0; i < 6; ++i) {
				if (!faceHidden(block->neighbors, i) && !isTransparentBlock(block->type)) {
					render->faceArray[i][opqIdx[i]][0] = (f32)block->x + (f32)(chunks->x * 16);
					render->faceArray[i][opqIdx[i]][1] = (f32)block->y + (f32)(subID * 16);
					render->faceArray[i][opqIdx[i]][2] = (f32)block->z + (f32)(chunks->z * 16);
					render->faceTypeID[i][opqIdx[i]] = s32StoreValues(block->type, i, chunks->biomeId, 0);
					if (chunks->x == 0 && chunks->z == 0 && subID == 0 && block->y == 0) {
						displayAllAtlasBlock(render->faceArray[i][opqIdx[i]][0], render->faceArray[i][opqIdx[i]][2], &render->faceTypeID[i][opqIdx[i]]);
					}
					opqIdx[i] += 1;
				} 
				// TOREFACT
				else if (!faceHidden(block->neighbors, i) && isTransparentBlock(block->type)) { /* Water face fill */
					render->trspFaceArray[i][trspIdx[i]][0] = (f32)block->x + (f32)(chunks->x * 16);
					render->trspFaceArray[i][trspIdx[i]][1] = (f32)block->y + (f32)(subID * 16);
					render->trspFaceArray[i][trspIdx[i]][2] = (f32)block->z + (f32)(chunks->z * 16);
					render->trspTypeId[i][trspIdx[i]] = s32StoreValues(block->type, i, chunks->biomeId, 0);
					trspIdx[i] += 1;
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
	// glBindVertexArray(0);
}



void allFaceDisplay(Context *c, u8 first) {
	HashMap_it		it;
	RenderChunks	*render = NULL;
    u32 			faceNb = 0;
	GLuint faceVBO = 0, faceTypeVBO = 0;

	for (u8 i = 0; i < 6; ++i) {
		glBindVertexArray(c->faceCube[i].VAO);
		it = hashmap_iterator(c->world->renderChunksMap);
		/* Basic face display */
		while (hashmap_next(&it)) {
			render = (RenderChunks *)it.value;
			c->displayData.faceRendered += faceNb;
			if (first) {
				faceNb = render->faceCount[i];
				faceVBO = render->faceVBO[i];
				faceTypeVBO = render->faceTypeVBO[i];
			} else {
				faceNb = render->trspFaceCount[i];
				faceVBO = render->trspFaceVBO[i];
				faceTypeVBO = render->trspTypeVBO[i];
			}
			drawFace(faceVBO, faceTypeVBO, 6U, faceNb);
		}
		/* Underground face display */
		if (first && c->world->undergroundBlock->isUnderground && c->displayUndergroundBlock) {
			drawFace(c->world->undergroundBlock->udgFaceVBO[i], c->world->undergroundBlock->udgTypeVBO[i], 6U, TOTAL_UNDERGROUND_FACE);			
		}
		glBindVertexArray(0);

	}
}

/**
 * @brief Draw all chunks by face
 * @param *c Context
*/
void drawAllChunksByFace(Context *c) {

    
    mtx_lock(&c->renderMtx);
	c->displayData.chunkRenderedNb = hashmap_size(c->world->renderChunksMap);
	c->displayData.faceRendered = 0;
	allFaceDisplay(c, TRUE);
	allFaceDisplay(c, FALSE);
    mtx_unlock(&c->renderMtx);
}