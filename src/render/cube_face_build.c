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

/* To call in create render chunk -> DONE */
void chunksCubeFaceGet(Mutex *chunkMtx, Chunks *chunks, RenderChunks *render)
{
	u32 idx[6] = {0};
	u32 waterFaceCount = 0;
    // s8	next = TRUE;


	ft_bzero(idx, sizeof(u32) * 6);

	// mtx_lock(chunkMtx);
	(void)chunkMtx;

	render->faceCount = faceVisibleCount(chunks, &waterFaceCount);
	for (u8 i = 0; i < 6; ++i) {
		render->faceArray[i] = ft_calloc(sizeof(vec3), render->faceCount[i]);
		render->faceTypeID[i] = ft_calloc(sizeof(f32), render->faceCount[i]);
	}

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
					idx[i] += 1;
				} else if (i == 5U && block->type == WATER) {
					render->topWaterFaceArray[waterFaceCount][0] = (f32)block->x + (f32)(chunks->x * 16);
					render->topWaterFaceArray[waterFaceCount][1] = (f32)block->y + (f32)(subID * 16);
					render->topWaterFaceArray[waterFaceCount][2] = (f32)block->z + (f32)(chunks->z * 16);
					render->topWaterTypeID[waterFaceCount] = (f32)block->type; // useless for now  but mandatory for shader can refact it
					waterFaceCount++;
				}
			}
		}
	}



	// mtx_unlock(chunkMtx);

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

void drawWaterFace(RenderChunks *render, u32 vertex_nb, u32 faceNb) {
	/* Bind Block instance VBO */
	
	glBindBuffer(GL_ARRAY_BUFFER, render->topWaterFaceVBO);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glVertexAttribDivisor(1, 1);

	/* Bind Block type VBO */
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, render->topWaterTypeVBO);
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
	
		if (i == 5U) {
			it = hashmap_iterator(c->world->renderChunksMap);
			while (hashmap_next(&it)) {
				render = (RenderChunks *)it.value;
				faceNb = render->topWaterFaceCount;
				faceRendernb += faceNb;
				drawWaterFace(render, 6U, faceNb);
				// chunkRenderNb++;
			}
		}
	
		glBindVertexArray(0);
    
	}
    mtx_unlock(&c->renderMtx);
    VOX_PROTECTED_LOG(c, RESET_LINE""GREEN"Chunk Rendered: %u, "ORANGE"Loaded %u "RESET""CYAN"In load %u "RESET""YELLOW"Visible Face: %u"RESET", "PINK" FPS: %d "RESET
    , chunkRenderNb / 6, c->chunkLoadedNb , c->chunkToLoadInQueue,faceRendernb, fpsGet());
}