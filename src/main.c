#include "../include/vox.h"			/* Main project header */
#include "../include/skybox.h"		/* skybox rendering */
#include "../include/render_chunks.h"
#include "../include/perlin_noise.h"

// u32 countChunksBlock(BlockPos chunkID, HashMap *chunksMap, Mutex *chunkMtx) {
// 	u32 totalBlock = 0;
// 	Chunks *chunks = NULL;

// 	mtx_lock(chunkMtx);
// 	chunks = hashmap_get(chunksMap, chunkID);
// 	if (chunks) {
// 		totalBlock = chunks->nb_block;
// 	}
// 	mtx_unlock(chunkMtx);
// 	return (totalBlock);
// }


// void drawAllChunks(Context *c, GLuint VAO) {
// 	s8			next = TRUE;
// 	u32 		chunkRenderNb = 0, blockRenderNb = 0;
// 	HashMap_it	it;
	
// 	mtx_lock(&c->renderMtx);
// 	it = hashmap_iterator(c->world->renderChunksMap);
// 	while ((next = hashmap_next(&it))) {
// 		RenderChunks *render = (RenderChunks *)it.value;
// 		drawAllCube(VAO, render);
// 		chunkRenderNb++;
// 		blockRenderNb += render->visibleBlock;
// 		// realTotalBlock += countChunksBlock(render->chunkID, c->world->chunksMap, &c->threadContext->chunkMtx);
// 	}
// 	ft_printf_fd(1, RESET_LINE""GREEN"Chunk Rendered: %u -> "YELLOW"Visible Block: %u"RESET","RESET""ORANGE" Loaded: %d, "RESET""CYAN" In loading: %d, "RESET""PINK" FPS: %d "RESET
// 	, chunkRenderNb, blockRenderNb, hashmap_size(c->world->chunksMap), hashmap_size(c->threadContext->chunksMapToLoad), fpsGet());
// 	mtx_unlock(&c->renderMtx);
// }

void chunksRender(Context *c, GLuint shader_id) {
    glLoadIdentity();
	glUseProgram(shader_id);
	// drawAllChunks(c, VAO);
	drawAllChunksByFace(c);
    glFlush();
}

void vox_destroy(Context *c) {
	s32 status = 0;
	mtx_lock(&c->gameMtx);
	c->isPlaying = FALSE;
	mtx_unlock(&c->gameMtx);
	
	thrd_join(c->threadContext->supervisor, &status);
	ft_printf_fd(1, PINK"\nSupervisor thread joined with status %d\n"RESET, status);

	hashmap_destroy(c->threadContext->chunksMapToLoad);
	free(c->threadContext->workers);

	mtx_destroy(&c->threadContext->chunkMtx);
	mtx_destroy(&c->renderMtx);
	mtx_destroy(&c->gameMtx);


	hashmap_destroy(c->world->renderChunksMap);
	hashmap_destroy(c->world->chunksMap);

	// free(c->threadContext->workers);
	free(c->threadContext);
	free(c->world);
	// free(c->cube.vertex);
	for (u32 i = 0; i < PERLIN_NOISE_HEIGHT; ++i) {
		free(c->perlin2D[i]);
	}
	free(c->perlin2D);
	free(c);
    // free(c->perlinNoise);
	glfwTerminate();
}

void renderChunksLoadNewVBO(Context *c) {
	RenderChunks *render = NULL;


	mtx_lock(&c->renderMtx);
	
	for (t_list *current = c->vboToCreate; current; current = current->next) {
		BlockPos chunkID = *(BlockPos *)current->content;
		render = renderChunkCreateFaceVBO(&c->threadContext->chunkMtx, c->world->chunksMap, chunkID);
		if (render) {
			hashmap_set_entry(c->world->renderChunksMap, chunkID, render);
		}
		
	}
	renderChunksVBODestroy(c);
	ft_lstclear(&c->vboToCreate, free);
	mtx_unlock(&c->renderMtx);
}

void main_loop(Context *c, GLuint skyTexture) {
    while (!glfwWindowShouldClose(c->win_ptr)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		/* Input handling */
		glfwPollEvents();
        handle_input(c);
		
		/* Update data */
		update_camera(c, c->cubeShaderID);
		// chunksViewHandling(c, c->world->renderChunksMap);
		renderChunksLoadNewVBO(c);
		
		/* Render logic */
        displaySkybox(c->skyboxVAO, skyTexture, c->skyboxShaderID, c->cam.projection, c->cam.view);
        chunksRender(c, c->cubeShaderID);

		glfwSwapBuffers(c->win_ptr);
    }
}

int main() {
    Context *context;

	if (!(context = contextInit())) {
		return (1);
	}

	/* Disable VSync to avoid fps locking */
	// glfwSwapInterval(0);

	// main_loop(context, context->cubeVAO, context->skyTexture);
	main_loop(context, context->skyTexture);
    vox_destroy(context);
    return (0);
}




/* We need special logic to destroy renderChunksCacheMap cause data store is shared with renderChunksMap */
// renderChunkCacheMapDestroy(c->world->renderChunksCacheMap, c->world->renderChunksMap);
// void renderChunkCacheMapDestroy(HashMap *renderChunkCache, HashMap *renderChunks) {
// 	HashMap_it	it = hashmap_iterator(renderChunkCache);
// 	t_list		*removeNodeList = NULL, *removeDataList = NULL;
// 	BlockPos	*tmpChunkID = NULL;
// 	s8			next = TRUE;

// 	while ((next = hashmap_next(&it))) {
// 		BlockPos chunkID = ((RenderChunks *)it.value)->chunkID;
// 		tmpChunkID = malloc(sizeof(BlockPos));
// 		ft_memcpy(tmpChunkID, &chunkID, sizeof(BlockPos));
// 		if (chunksIsRenderer(renderChunks, chunkID)) {
// 			ft_lstadd_front(&removeNodeList, ft_lstnew(tmpChunkID));
// 		} else {
// 			ft_lstadd_front(&removeDataList, ft_lstnew(tmpChunkID));
// 		}
// 	}

// 	for (t_list *current = removeNodeList; current; current = current->next) {
// 		hashmap_remove_entry(renderChunkCache, *(BlockPos *)current->content, HASHMAP_FREE_NODE);
// 	}

// 	for (t_list *current = removeDataList; current; current = current->next) {
// 		hashmap_remove_entry(renderChunkCache, *(BlockPos *)current->content, HASHMAP_FREE_DATA);
// 	}
// 	ft_lstclear(&removeNodeList, free);
// 	ft_lstclear(&removeDataList, free);

// 	hashmap_destroy(renderChunkCache);
// }