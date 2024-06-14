#include "../include/vox.h"			/* Main project header */
#include "../include/skybox.h"		/* skybox rendering */
#include "../include/render_chunks.h"
#include "../include/perlin_noise.h"
#include "../include/thread_load.h"

void chunksRender(Context *c, GLuint shader_id) {
    glLoadIdentity();
	glUseProgram(shader_id);
	drawAllChunksByFace(c);
    glFlush();
}

void renderChunksVBODestroy(Context *c) {
	if (mtx_trylock(&c->vboToDestroyMtx) != thrd_success) {
		renderNeedDataSet(c, TRUE);
		return ;
	}
	// mtx_lock(&c->vboToDestroyMtx);
	for (t_list *current = c->vboToDestroy; current; current = current->next) {
		glDeleteBuffers(1, (GLuint *)current->content);
	}
	ft_lstclear(&c->vboToDestroy, free);
	mtx_unlock(&c->vboToDestroyMtx);
}

void renderChunksLoadNewVBO(Context *c) {

	// /* LOCK chunk MTX */
	if (mtx_trylock(&c->threadContext->chunkMtx) != thrd_success) {
		renderNeedDataSet(c, TRUE);
		return ;
	}
	c->chunkLoadedNb = hashmap_size(c->world->chunksMap);
	c->chunkToLoadInQueue = hashmap_size(c->threadContext->chunksMapToLoad);
	mtx_lock(&c->vboToCreateMtx);
	for (t_list *current = c->vboToCreate; current; current = current->next) {
		BlockPos chunkID = *(BlockPos *)current->content;
		renderChunkCreateFaceVBO(c->world->chunksMap, chunkID);
	}
	renderNeedDataSet(c, FALSE);
	/* UNLOCK chunk MTX */
	mtx_unlock(&c->threadContext->chunkMtx);
	ft_lstclear(&c->vboToCreate, free);
	mtx_unlock(&c->vboToCreateMtx);
}

void renderChunksVBOhandling(Context *c) {
	renderChunksLoadNewVBO(c);
	renderChunksVBODestroy(c);
}

void vox_destroy(Context *c) {
	s32 status = 0;
	mtx_lock(&c->isRunningMtx);
	c->isPlaying = FALSE;
	mtx_unlock(&c->isRunningMtx);
	
	renderNeedDataSet(c, FALSE);

	thrd_join(c->threadContext->supervisor, &status);
	hashmap_destroy(c->threadContext->chunksMapToLoad);
	mtx_destroy(&c->threadContext->chunkMtx);
	// free(c->threadContext->workers);
	free(c->threadContext);

	ft_printf_fd(1, PINK"\nSupervisor thread joined with status %d\n"RESET, status);


	if (c->world->undergroundBlock->udgFaceCount != 0) {
		undergroundBlockFree(c->world->undergroundBlock);
		c->world->undergroundBlock->udgFaceCount = 0;
	}
	free(c->world->undergroundBlock);

	renderChunksVBODestroy(c);
	ft_lstclear(&c->vboToCreate, free);

	mtx_destroy(&c->renderMtx);
	mtx_destroy(&c->gameMtx);
	mtx_destroy(&c->isRunningMtx);
	mtx_destroy(&c->vboToDestroyMtx);
	mtx_destroy(&c->vboToCreateMtx);
	mtx_destroy(&c->renderDataNeededMtx);


	hashmap_destroy(c->world->renderChunksMap);
	hashmap_destroy(c->world->chunksMap);
	free(c->world);
	for (u32 i = 0; i < PERLIN_NOISE_HEIGHT; ++i) {
		free(c->perlin2D[i]);
	}
	free(c->perlin2D);
	for (u32 i = 0; i < PERLIN_SNAKE_HEIGHT; ++i) {
		free(c->perlinCaveNoise[i]);
	}
	free(c->perlinCaveNoise);
	for (u32 i = 0; i < FACE_VERTEX_ARRAY_SIZE; ++i) {
		glDeleteVertexArrays(1, &c->faceCube[i].VAO);
		glDeleteBuffers(1, &c->faceCube[i].VBO);
		glDeleteBuffers(1, &c->faceCube[i].EBO);
	}
	free(c->faceCube);
	free(c);
    // free(c->perlinNoise);
	glfwTerminate();
}


void updateGame(Context *c) {
	/* Input handling */
	glfwPollEvents();

	/* Update data */
	mtx_lock(&c->gameMtx);
	// mtxLockUpdateTime(c, &c->gameMtx, &c->mtxTime.start, &c->mtxTime.end, &c->mtxTime.gameMtxTime, "Game");

	handle_input(c);
	update_camera(c, c->cubeShaderID);

	mtx_unlock(&c->gameMtx);
}

void renderGame(Context *c, GLuint skyTexture) {
	/* Update render */
	renderChunksVBOhandling(c);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	/* Render logic */
	displaySkybox(c->skyboxVAO, skyTexture, c->skyboxShaderID, c->cam.projection, c->cam.view);
	chunksRender(c, c->cubeShaderID);
	glfwSwapBuffers(c->win_ptr);
}


void main_loop(Context *c, GLuint skyTexture) {
    while (!glfwWindowShouldClose(c->win_ptr)) {
		updateGame(c);
		renderGame(c, skyTexture);	
    }
}

int main() {
    Context *context;

	if (!(context = contextInit())) {
		return (1);
	}

	/* Disable VSync to avoid fps locking */
	// glfwSwapInterval(0);
	// testUpdateLogic(context, context->skyTexture);

	main_loop(context, context->skyTexture);
    vox_destroy(context);
    return (0);
}


// void testUpdateLogic(Context *c, GLuint skyTexture) {
//     double lastTime = glfwGetTime();
//     double timer = lastTime;
//     double deltaTime = 0;
//     double nowTime = 0;
//     int frames = 0;
//     int updates = 0;

//     double limitFPS = 60.0;

//     while (!glfwWindowShouldClose(c->win_ptr)) {
//         nowTime = glfwGetTime();
//         deltaTime += (nowTime - lastTime) * limitFPS; // Multiply instead of divide
//         lastTime = nowTime;
//         // Input, AI, Physics, etc.
//         while (deltaTime >= 1.0) {
//             updateGame(c); // Update game logic here
//             updates++;
//             deltaTime--;
//         }
//         // Rendering
//         renderGame(c, skyTexture); // Render game here
//         frames++;

//         // Reset after one second
//         if (glfwGetTime() - timer > 1.0) {
//             timer++;
//             VOX_PROTECTED_LOG(c, RESET_LINE""PINK"FPS: %d, Updates: %d\n"RESET, frames, updates);
//             (void)frames, (void)updates;
//             updates = 0;
//             frames = 0;
//         }
//     }
// }






/* We need special logic to destroy renderChunksCacheMap cause data store is shared with renderChunksMap */
// renderChunkCacheMapDestroy(c->world->renderChunksCacheMap, c->world->renderChunksMap);
// void renderChunkCacheMapDestroy(HashMap *renderChunkCache, HashMap *renderChunks) {
// 	HashMap_it	it = hashmap_iterator(renderChunkCache);
// 	t_list		*removeNodeList = NULL, *removeDataList = NULL;
// 	BlockPos	*tmpChunkID = NULL;
// 	s8			next = TRUE;

// 	while (hashmap_next(&it)) {
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