#include "../include/vox.h"			/* Main project header */
#include "../include/skybox.h"		/* skybox rendering */
#include "../include/render_chunks.h"
#include "../include/perlin_noise.h"

void chunksRender(Context *c, GLuint shader_id) {
    glLoadIdentity();
	glUseProgram(shader_id);
	drawAllChunksByFace(c);
    glFlush();
}

void vox_destroy(Context *c) {
	s32 status = 0;
	mtx_lock(&c->gameMtx);
	c->isPlaying = FALSE;
	mtx_unlock(&c->gameMtx);
	
	thrd_join(c->threadContext->supervisor, &status);
	hashmap_destroy(c->threadContext->chunksMapToLoad);
	free(c->threadContext->workers);
	mtx_destroy(&c->threadContext->chunkMtx);
	free(c->threadContext);

	ft_printf_fd(1, PINK"\nSupervisor thread joined with status %d\n"RESET, status);

	renderChunksVBODestroy(c);
	ft_lstclear(&c->vboToCreate, free);

	mtx_destroy(&c->renderMtx);
	mtx_destroy(&c->gameMtx);



	hashmap_destroy(c->world->renderChunksMap);
	hashmap_destroy(c->world->chunksMap);
	free(c->world);
	for (u32 i = 0; i < PERLIN_NOISE_HEIGHT; ++i) {
		free(c->perlin2D[i]);
		free(c->perlinCaveNoise[i]);
	}
	free(c->perlin2D);
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


void updateGame(Context *c) {
		/* Input handling */
		glfwPollEvents();
        handle_input(c);
		
		/* Update data */
		update_camera(c, c->cubeShaderID);
		renderChunksLoadNewVBO(c);
}

void renderGame(Context *c, GLuint skyTexture) {
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
//             printf("FPS: %d, Updates: %d\n", frames, updates);
//             (void)frames, (void)updates;
//             updates = 0;
//             frames = 0;
//         }
//     }
// }


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