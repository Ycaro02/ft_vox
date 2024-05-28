#include "../../include/vox.h"			/* Main project header */
#include "../../include/chunks.h"
#include "../../include/perlin_noise.h"
#include "../../include/render_chunks.h"
#include "../../include/thread_load.h"

s8 faceHidden(u8 neighbors, u8 face) {
	return (neighbors & (1U << face));
}

u32 *faceVisibleCount(Chunks *chunks) {
	u32 *count = ft_calloc(sizeof(u32), 6);
    s8 	next = TRUE;

	for (s32 subID = 0; chunks->sub_chunks[subID].block_map != NULL; ++subID) {
		HashMap_it it = hashmap_iterator(chunks->sub_chunks[subID].block_map);
		while ((next = hashmap_next(&it))) {
			Block *block = (Block *)it.value;
			for (u8 i = 0; i < 6; ++i) {
				if (!faceHidden(block->neighbors, i)) {
					count[i] += 1U;
				}
			}
		}
	}
	return (count);
}

/* To call in create render chunk -> DONE */
void chunksCubeFaceGet(Chunks *chunks, RenderChunks *render)
{
	u32 idx[6] = {0};
    s8	next = TRUE;

	ft_bzero(idx, sizeof(u32) * 6);

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
				if (!faceHidden(block->neighbors, i)) {
					render->faceArray[i][idx[i]][0] = (f32)block->x + (f32)(chunks->x * 16);
					render->faceArray[i][idx[i]][1] = (f32)block->y + (f32)(subID * 16);
					render->faceArray[i][idx[i]][2] = (f32)block->z + (f32)(chunks->z * 16);
					render->faceTypeID[i][idx[i]] = (f32)block->type;
					idx[i] += 1;
				}
			}
		}
	}
}


GLuint faceInstanceVBOCreate(vec3 *faceArray, u32 faceNb) {
    return (bufferGlCreate(GL_ARRAY_BUFFER, faceNb * sizeof(vec3), (void *)faceArray[0]));
}

/* TO CALL in main thread -> DONE */
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

/* NEW draw logic */

void drawFace(GLuint VAO, RenderChunks *render, u32 vertex_nb, u32 faceNb, u8 faceIdx) {
	// glBindVertexArray(VAO);
	(void)VAO;

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

u32 countChunksBlock(BlockPos chunkID, HashMap *chunksMap, Mutex *chunkMtx) {
	u32 totalBlock = 0;
	Chunks *chunks = NULL;

	mtx_lock(chunkMtx);
	chunks = hashmap_get(chunksMap, chunkID);
	if (chunks) {
		totalBlock = chunks->nb_block;
	}
	mtx_unlock(chunkMtx);
	return (totalBlock);
}


/* TO CALL in chunksRender in main  -> DONE */
void drawAllChunksByFace(Context *c) {
    s8			next = TRUE;
    u32 		chunkRenderNb = 0, blockRenderNb = 0, faceRendernb = 0, totalBlock = 0;
    HashMap_it	it;
    
    mtx_lock(&c->renderMtx);
    for (u8 i = 0; i < 6; ++i) {
		glBindVertexArray(c->faceCube[i].VAO);

        it = hashmap_iterator(c->world->renderChunksMap);
        while ((next = hashmap_next(&it))) {
            RenderChunks *render = (RenderChunks *)it.value;
            u32 faceNb = render->faceCount[i];
            // u32 faceVertexNb = (faceNb * 4U);
            drawFace(c->faceCube[i].VAO, render, 6U, faceNb, i);
            chunkRenderNb++;
            blockRenderNb += render->visibleBlock;
			faceRendernb += faceNb;
			totalBlock += countChunksBlock(render->chunkID, c->world->chunksMap, &c->threadContext->chunkMtx);
        }
		glBindVertexArray(0);
    
	}
    ft_printf_fd(1, RESET_LINE""GREEN"Chunk Rendered: %u -> "RED"Total Block: %u "RESET" "YELLOW"Visible Block: %u, Face: %u"RESET","RESET""ORANGE" Loaded: %d, "RESET""CYAN" In loading: %d, "RESET""PINK" FPS: %d "RESET
    , chunkRenderNb / 6, totalBlock / 6, blockRenderNb / 6, faceRendernb, hashmap_size(c->world->chunksMap), hashmap_size(c->threadContext->chunksMapToLoad), fpsGet());
    mtx_unlock(&c->renderMtx);
}