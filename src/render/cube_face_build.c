#include "../../include/vox.h"			/* Main project header */
#include "../../include/chunks.h"
#include "../../include/perlin_noise.h"
#include "../../include/render_chunks.h"
#include "../../include/thread_load.h"

s8 faceVisible(u8 neighbors, u8 face) {
	return (neighbors & (1 << face));
}

u32 *faceVisibleCount(Chunks *chunks) {
	u32 *count = ft_calloc(sizeof(u32), 6);
    s8 	next = TRUE;

	for (s32 subID = 0; chunks->sub_chunks[subID].block_map != NULL; ++subID) {
		HashMap_it it = hashmap_iterator(chunks->sub_chunks[subID].block_map);
		while ((next = hashmap_next(&it))) {
			Block *block = (Block *)it.value;
			for (u8 i = 0; i < 6; ++i) {
				if (faceVisible(block->neighbors, i)) {
					count[i] += 1U;
				}
			}
		}
	}
	return (count);
}

/* To call in create render chunk */
void chunksCubeFaceGet(Chunks *chunks, RenderChunks *render)
{
	u32 idx = 0;
    s8	next = TRUE;

	render->faceCount = faceVisibleCount(chunks);
	for (u8 i = 0; i < 6; ++i) {
		render->faceArray[i] = ft_calloc(sizeof(vec3), render->faceCount[i]);
		render->faceTypeID[i] = ft_calloc(sizeof(f32), render->faceCount[i]);
	}


	for (s32 subID = 0; chunks->sub_chunks[subID].block_map != NULL; ++subID) {
		HashMap_it it = hashmap_iterator(chunks->sub_chunks[subID].block_map);
		while ((next = hashmap_next(&it))) {
			Block *block = (Block *)it.value;
			for (u8 i = 0; i < 6; ++i) {
				if (faceVisible(block->neighbors, i)) {
					render->faceArray[i][idx][0] = (f32)block->x + (f32)(chunks->x * 16);
					render->faceArray[i][idx][1] = (f32)block->y + (f32)(subID * 16);
					render->faceArray[i][idx][2] = (f32)block->z + (f32)(chunks->z * 16);
					render->faceTypeID[i][idx] = (f32)block->type;
					++idx;
				}
			}
		}
	}
}


/* TO CALL in main thread */
GLuint faceInstanceVBOCreate(vec3 *faceArray, u32 visibleBlock) {
    return (bufferGlCreate(GL_ARRAY_BUFFER, visibleBlock * sizeof(vec3), (void *)faceArray[0]));
}

RenderChunks *renderChunkCreateFaceVBO(Mutex *chunkMtx, HashMap *chunksMap, BlockPos chunkID) {
	/* Create VBO */
	Chunks 			*chunks = NULL; 
	RenderChunks 	*render = NULL; 


	mtx_lock(chunkMtx);

	chunks = hashmap_get(chunksMap, chunkID);
	if (!chunks || !chunks->render) {
		mtx_unlock(chunkMtx);
		return (NULL);
	}

	render = chunks->render;
	for (u8 i = 0; i < 6; ++i) {
		render->faceVBO[i] = faceInstanceVBOCreate(render->faceArray[i], render->faceCount[i]);
		render->faceTypeVBO[i] = bufferGlCreate(GL_ARRAY_BUFFER, render->faceCount[i] * sizeof(GLuint), (void *)&render->faceTypeID[i][0]);
	}

	mtx_unlock(chunkMtx);
	return (render);
}

/* Need to implement new draw logic read face and faceModel */

void drawFace(GLuint VAO, RenderChunks *render, u32 vertex_nb, u32 cubeId, u8 faceIdx) {
	glBindVertexArray(VAO);

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

	glDrawElementsInstanced(GL_TRIANGLES, vertex_nb, GL_UNSIGNED_INT, 0, cubeId);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}


void drawAllFace(GLuint VAO, RenderChunks *render) {
	u32 faceNb = 0, faceVertexNb = 0;

	for (u8 i = 0; i < 6; ++i) {
		faceNb = render->faceCount[i];
		faceVertexNb = faceNb * 6;
		drawFace(VAO, render, faceVertexNb, faceNb, i);
	}
}

/* TO CALL in chunksRender in main */
void drawAllChunksByFace(Context *c, GLuint VAO) {
	s8			next = TRUE;
	u32 		chunkRenderNb = 0, blockRenderNb = 0;
	HashMap_it	it;
	
	mtx_lock(&c->renderMtx);
	it = hashmap_iterator(c->world->renderChunksMap);
	while ((next = hashmap_next(&it))) {
		RenderChunks *render = (RenderChunks *)it.value;
		drawAllFace(VAO, render);
		chunkRenderNb++;
		blockRenderNb += render->visibleBlock;
		// realTotalBlock += countChunksBlock(render->chunkID, c->world->chunksMap, &c->threadContext->chunkMtx);
	}
	ft_printf_fd(1, RESET_LINE""GREEN"Chunk Rendered: %u -> "YELLOW"Visible Block: %u"RESET","RESET""ORANGE" Loaded: %d, "RESET""CYAN" In loading: %d, "RESET""PINK" FPS: %d "RESET
	, chunkRenderNb, blockRenderNb, hashmap_size(c->world->chunksMap), hashmap_size(c->threadContext->chunksMapToLoad), fpsGet());
	mtx_unlock(&c->renderMtx);
}