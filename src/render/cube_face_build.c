#include "../../include/vox.h"			/* Main project header */
#include "../../include/world.h"
#include "../../include/chunks.h"
#include "../../rsc/perlin_noise/include/perlin_noise.h"
#include "../../include/render_chunks.h"
#include "../../include/thread_load.h"
#include "../../include/block.h"
#include "../../include/cube.h"

s8 isTransparentBlock(u8 type) {
	return (type == WATER || type == ICE);
}

s8 faceHidden(u8 neighbors, u8 face) {
	return (neighbors & (1U << face));
}

u32 *faceVisibleCount(Chunks *chunks, u32 *transparentFaceCount) {
	u32 *count = ft_calloc(sizeof(u32), 6);
    // s8 	next = TRUE;


	for (s32 subID = 0; chunks->sub_chunks[subID].block_map != NULL; ++subID) {
		HashMap_it it = hashmap_iterator(chunks->sub_chunks[subID].block_map);
		while (hashmap_next(&it)) {
			Block *block = (Block *)it.value;
			for (u8 i = 0; i < 6; ++i) {
				if (!faceHidden(block->neighbors, i) && !isTransparentBlock(block->type)) {
					count[i] += 1U;
				}
				if (isTransparentBlock(block->type) && i == 5U && !faceHidden(block->neighbors, i)) {
					*transparentFaceCount += 1U;
				}
			}
		}
	}

	return (count);
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
	u32 idx[6] = {0};
	u32 transparentFaceCount = 0;
	u32 count = 0;

	ft_bzero(idx, sizeof(u32) * 6);

	(void)chunkMtx;

	render->faceCount = faceVisibleCount(chunks, &transparentFaceCount);
	for (u8 i = 0; i < 6; ++i) {
		render->faceArray[i] = ft_calloc(sizeof(vec3), render->faceCount[i]);
		render->faceTypeID[i] = ft_calloc(sizeof(f32), render->faceCount[i]);
	}

	/* Water face init */
	render->topTransparencyFaceArray = ft_calloc(sizeof(vec3), transparentFaceCount);
	render->topTransparencyTypeId = ft_calloc(sizeof(f32), transparentFaceCount);
	render->topTransparencyCount = transparentFaceCount;

	for (s32 subID = 0; chunks->sub_chunks[subID].block_map != NULL; ++subID) {
		HashMap_it it = hashmap_iterator(chunks->sub_chunks[subID].block_map);
		while (hashmap_next(&it)) {
			Block *block = (Block *)it.value;
			for (u8 i = 0; i < 6; ++i) {
				if (!faceHidden(block->neighbors, i) && !isTransparentBlock(block->type)) {
					render->faceArray[i][idx[i]][0] = (f32)block->x + (f32)(chunks->x * 16);
					render->faceArray[i][idx[i]][1] = (f32)block->y + (f32)(subID * 16);
					render->faceArray[i][idx[i]][2] = (f32)block->z + (f32)(chunks->z * 16);
					render->faceTypeID[i][idx[i]] = s32StoreValues(block->type, i, 0, 0);
					if (chunks->x == 0 && chunks->z == 0 && subID == 0 && block->y == 0) {
						displayAllAtlasBlock(render->faceArray[i][idx[i]][0], render->faceArray[i][idx[i]][2], &render->faceTypeID[i][idx[i]]);
					}
					idx[i] += 1;
				} else if (i == 5U && isTransparentBlock(block->type)) { /* Water face fill */
					render->topTransparencyFaceArray[count][0] = (f32)block->x + (f32)(chunks->x * 16);
					render->topTransparencyFaceArray[count][1] = (f32)block->y + (f32)(subID * 16);
					render->topTransparencyFaceArray[count][2] = (f32)block->z + (f32)(chunks->z * 16);
					render->topTransparencyTypeId[count] = s32StoreValues(block->type, i, 0, 0);
					count++;
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
	}

	render->topTransparencyFaceVBO = faceInstanceVBOCreate(render->topTransparencyFaceArray, render->topTransparencyCount);
	render->topTransparencyTypeVBO = bufferGlCreate(GL_ARRAY_BUFFER, render->topTransparencyCount * sizeof(GLuint), (void *)&render->topTransparencyTypeId[0]);
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

/**
 * @brief Draw all chunks by face
 * @param *c Context
*/
void drawAllChunksByFace(Context *c) {
    HashMap_it		it;
	RenderChunks	*render = NULL;
    u32 			faceNb = 0;
    
    mtx_lock(&c->renderMtx);
	c->displayData.chunkRenderedNb = hashmap_size(c->world->renderChunksMap);
	c->displayData.faceRendered = 0;
    for (u8 i = 0; i < 6; ++i) {
		glBindVertexArray(c->faceCube[i].VAO);
        it = hashmap_iterator(c->world->renderChunksMap);
		/* Basic face display */
        while (hashmap_next(&it)) {
            render = (RenderChunks *)it.value;
            faceNb = render->faceCount[i];
            c->displayData.faceRendered += faceNb;
			drawFace(render->faceVBO[i], render->faceTypeVBO[i], 6U, faceNb);
        }
		/* Underground face display */
		if (c->world->undergroundBlock->isUnderground && c->displayUndergroundBlock) {
			drawFace(c->world->undergroundBlock->udgFaceVBO[i], c->world->undergroundBlock->udgTypeVBO[i], 6U, TOTAL_UNDERGROUND_FACE);			
		}
		/* Transparency top face display */
		if (i == (u32)TOP_FACE) {
			it = hashmap_iterator(c->world->renderChunksMap);
			while (hashmap_next(&it)) {
				render = (RenderChunks *)it.value;
				faceNb = render->topTransparencyCount;
				c->displayData.faceRendered += faceNb;
				drawFace(render->topTransparencyFaceVBO, render->topTransparencyTypeVBO, 6U, faceNb);
			}
		}
		glBindVertexArray(0);
    
	}
    mtx_unlock(&c->renderMtx);
}