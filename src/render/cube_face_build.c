#include "../../include/vox.h"			/* Main project header */
#include "../../include/chunks.h"
#include "../../include/perlin_noise.h"
#include "../../include/render_chunks.h"
#include "../../include/thread_load.h"

s8 faceHidden(u8 neighbors, u8 face) {
	return (neighbors & (1U << face));
}

u32 *faceVisibleCount(Chunks *chunks, u32 *waterFaceCount) {
	u32 *count = ft_calloc(sizeof(u32), 6);
    // s8 	next = TRUE;


	for (s32 subID = 0; chunks->sub_chunks[subID].block_map != NULL; ++subID) {
		HashMap_it it = hashmap_iterator(chunks->sub_chunks[subID].block_map);
		while (hashmap_next(&it)) {
			Block *block = (Block *)it.value;
			for (u8 i = 0; i < 6; ++i) {
				if (!faceHidden(block->neighbors, i) && block->type != WATER) {
					count[i] += 1U;
				}
				if (block->type == WATER && i == 5U && !faceHidden(block->neighbors, i)) {
					*waterFaceCount += 1U;
				}
			}
		}
	}

	return (count);
}

void woolHelpDebug(f32 x, f32 z, f32 *type) {
	if ((x == 0 || x == 8) && (z == 0 || z == 1)) {
		*type = (f32)WOOL_RED;
	} else if ((x == 1 || x == 9) && (z == 0 || z == 1)) {
		*type = (f32)WOOL_BLUE;
	} else if ((x == 2 || x == 10) && (z == 0 || z == 1)) {
		*type = (f32)WOOL_GREEN;
	} else if ((x == 3 || x == 11) && (z == 0 || z == 1)) {
		*type = (f32)WOOL_CYAN;
	} else if ((x == 4 || x == 12) && (z == 0 || z == 1)) {
		*type = (f32)WOOL_LIGHT_BLUE;
	} else if ((x == 5 || x == 13) && (z == 0 || z == 1)) {
		*type = (f32)WOOL_LIGHTGREEN;
	} else if ((x == 6 || x == 14) && (z == 0 || z == 1)) {
		*type = (f32)WOOL_YELLOW;
	} else if ((x == 7 || x == 15) && (z == 0 || z == 1)) {
		*type = (f32)WOOL_ORANGE;
	}
}

/* To call in create render chunk -> DONE */
void chunksCubeFaceGet(Mutex *chunkMtx, Chunks *chunks, RenderChunks *render)
{
	u32 idx[6] = {0};
	u32 waterFaceCount = 0;

	ft_bzero(idx, sizeof(u32) * 6);

	(void)chunkMtx;

	render->faceCount = faceVisibleCount(chunks, &waterFaceCount);
	for (u8 i = 0; i < 6; ++i) {
		render->faceArray[i] = ft_calloc(sizeof(vec3), render->faceCount[i]);
		render->faceTypeID[i] = ft_calloc(sizeof(f32), render->faceCount[i]);
	}

	/* Water face init */
	render->topWaterFaceArray = ft_calloc(sizeof(vec3), waterFaceCount);
	render->topWaterTypeID = ft_calloc(sizeof(f32), waterFaceCount);
	render->topWaterFaceCount = waterFaceCount;
	waterFaceCount = 0;

	for (s32 subID = 0; chunks->sub_chunks[subID].block_map != NULL; ++subID) {
		HashMap_it it = hashmap_iterator(chunks->sub_chunks[subID].block_map);
		while (hashmap_next(&it)) {
			Block *block = (Block *)it.value;
			for (u8 i = 0; i < 6; ++i) {
				if (!faceHidden(block->neighbors, i) && block->type != WATER) {
					render->faceArray[i][idx[i]][0] = (f32)block->x + (f32)(chunks->x * 16);
					render->faceArray[i][idx[i]][1] = (f32)block->y + (f32)(subID * 16);
					render->faceArray[i][idx[i]][2] = (f32)block->z + (f32)(chunks->z * 16);
					render->faceTypeID[i][idx[i]] = (f32)block->type;
					woolHelpDebug(render->faceArray[i][idx[i]][0], render->faceArray[i][idx[i]][2], &render->faceTypeID[i][idx[i]]);
					idx[i] += 1;
				} else if (i == 5U && block->type == WATER) { /* Water face fill */
					render->topWaterFaceArray[waterFaceCount][0] = (f32)block->x + (f32)(chunks->x * 16);
					render->topWaterFaceArray[waterFaceCount][1] = (f32)block->y + (f32)(subID * 16);
					render->topWaterFaceArray[waterFaceCount][2] = (f32)block->z + (f32)(chunks->z * 16);
					render->topWaterTypeID[waterFaceCount] = (f32)block->type; // useless for now  but mandatory for shader can refact it
					waterFaceCount++;
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

	render->topWaterFaceVBO = faceInstanceVBOCreate(render->topWaterFaceArray, render->topWaterFaceCount);
	render->topWaterTypeVBO = bufferGlCreate(GL_ARRAY_BUFFER, render->topWaterFaceCount * sizeof(GLuint), (void *)&render->topWaterTypeID[0]);
}


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

#define UNDERGROUND_FACE_NB 2
#define UNDERGROUND_INCREMENT 1.0f


void underGroundBlockArrayFillAxis(vec3 *faceArray, f32 *faceTypeID, vec3 camPos, f32 incrementX) {

	for (u32 i = 0; i < UNDERGROUND_FACE_NB; ++i) {
		faceArray[i][0] = (camPos[0] * 2.0f) + incrementX;
		faceArray[i][1] = (camPos[1] * 2.0f);
		faceArray[i][2] = (camPos[2] * 2.0f);
		faceTypeID[i] = (f32)STONE;
		incrementX += UNDERGROUND_INCREMENT;
	}
}

void undergroundBlockcreate(Context *c) {
	if (c->world->undergroundBlock->isUnderground == FALSE) {
		return ;
	} 
	mtx_lock(&c->gameMtx);
	vec3 camPos = {0};
	glm_vec3_copy(c->cam.position, camPos);
	mtx_unlock(&c->gameMtx);

	if (c->world->undergroundBlock->udgFaceCount != 0) {
		undergroundBlockFree(c->world->undergroundBlock);
		c->world->undergroundBlock->udgFaceCount = 0;
	}

	UndergroundBlock *udg = c->world->undergroundBlock;

	for (u8 i = 0; i < 6; ++i) {
		udg->udgFaceArray[i] = ft_calloc(sizeof(vec3), UNDERGROUND_FACE_NB);
		udg->udgTypeID[i] = ft_calloc(sizeof(vec3), UNDERGROUND_FACE_NB);
		underGroundBlockArrayFillAxis(udg->udgFaceArray[i], udg->udgTypeID[i], camPos, -0.5f);
		udg->udgFaceVBO[i] = faceInstanceVBOCreate(udg->udgFaceArray[i], UNDERGROUND_FACE_NB);
		udg->udgTypeVBO[i] = bufferGlCreate(GL_ARRAY_BUFFER, UNDERGROUND_FACE_NB * sizeof(GLuint), (void *)udg->udgTypeID[i]);
		udg->udgFaceCount += UNDERGROUND_FACE_NB;
	}
}

/* NEW draw logic */

void drawFace(RenderChunks *render, u32 vertex_nb, u32 faceNb, u8 faceIdx) {
	/* Bind Block instance VBO */
	glBindBuffer(GL_ARRAY_BUFFER, render->faceVBO[faceIdx]);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glVertexAttribDivisor(1, 1);
	
	/* Bind Block type VBO */
    glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, render->faceTypeVBO[faceIdx]);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
	glVertexAttribDivisor(3, 1);

	glDrawElementsInstanced(GL_TRIANGLES, vertex_nb, GL_UNSIGNED_INT, 0, faceNb);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
	// glBindVertexArray(0);
}

void drawSpecialFace(GLuint faceVBO, GLuint typeVBO, u32 vertex_nb, u32 faceNb) {
	/* Bind Block instance VBO */
	
	glBindBuffer(GL_ARRAY_BUFFER, faceVBO);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glVertexAttribDivisor(1, 1);

	/* Bind Block type VBO */
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, typeVBO);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
	glVertexAttribDivisor(3, 1);
	
	glDrawElementsInstanced(GL_TRIANGLES, vertex_nb, GL_UNSIGNED_INT, 0, faceNb);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// glBindVertexArray(0);
}

/* TO CALL in chunksRender in main  -> DONE */
void drawAllChunksByFace(Context *c) {
    HashMap_it	it;
    // s8			next = TRUE;
	RenderChunks *render = NULL;
	u32 faceNb = 0;

    u32 		chunkRenderNb = 0, faceRendernb = 0;
    
	undergroundBlockcreate(c);

    mtx_lock(&c->renderMtx);
    for (u8 i = 0; i < 6; ++i) {
		glBindVertexArray(c->faceCube[i].VAO);

        it = hashmap_iterator(c->world->renderChunksMap);
        while (hashmap_next(&it)) {
            render = (RenderChunks *)it.value;
            faceNb = render->faceCount[i];
            faceRendernb += faceNb;
            drawFace(render, 6U, faceNb, i);
            chunkRenderNb++;
        }
	
		if (c->world->undergroundBlock->isUnderground) { /* Front face*/
			drawSpecialFace(c->world->undergroundBlock->udgFaceVBO[i], c->world->undergroundBlock->udgTypeVBO[i], 6U, UNDERGROUND_FACE_NB);			
		}

		if (i == 5U) { /* TOP face */
			it = hashmap_iterator(c->world->renderChunksMap);
			while (hashmap_next(&it)) {
				render = (RenderChunks *)it.value;
				faceNb = render->topWaterFaceCount;
				faceRendernb += faceNb;
				drawSpecialFace(render->topWaterFaceVBO, render->topWaterTypeVBO, 6U, faceNb);
				// chunkRenderNb++;
			}
		}
	
		glBindVertexArray(0);
    
	}
    mtx_unlock(&c->renderMtx);
    VOX_PROTECTED_LOG(c, RESET_LINE""GREEN"Chunk Rendered: %u, "ORANGE"Loaded %u "RESET""CYAN"In load %u "RESET""YELLOW"Visible Face: %u"RESET", "PINK" FPS: %d "RESET
    , chunkRenderNb / 6, c->chunkLoadedNb , c->chunkToLoadInQueue,faceRendernb, fpsGet());
}