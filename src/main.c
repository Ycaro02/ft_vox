// #include <ft2build.h>
// #include FT_FREETYPE_H
#include "../include/text_render.h" /* text rendering, need to include this first  */
#include "../include/vox.h"			/* Main project header */
#include "../include/world.h"
#include "../include/skybox.h"		/* skybox rendering */
#include "../include/render_chunks.h"
#include "../include/perlin_noise.h"
#include "../include/thread_load.h"
#include "../include/block.h"
#include "../include/cube.h"
#include "../include/chunks.h"
#include "../include/camera.h"
#include "../include/win_event.h"
#include "../include/window.h"

void chunksRender(Context *c, GLuint shader_id) {
    // glLoadIdentity();
	glUseProgram(shader_id);
	drawAllChunksByFace(c);
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

	undergroundBlockcreate(c);


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
	free(c->threadContext);

	ft_printf_fd(1, PINK"Supervisor thread joined with status %d\n"RESET, status);
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

	glDeleteVertexArrays(1, &c->skyboxVAO);

	glDeleteProgram(c->cubeShaderID);
	glDeleteProgram(c->skyboxShaderID);

	glDeleteTextures(1, &c->skyTexture);
	glDeleteTextures(1, &c->blockAtlasId);

	for (u32 i = 0; i < 128; i++) {
		glDeleteTextures(1, &c->fontContext->font[i].TextureID);
	}

	glDeleteProgram(c->fontContext->fontShaderID);
	glDeleteVertexArrays(1, &c->fontContext->VAO);
	glDeleteBuffers(1, &c->fontContext->VBO);

	free(c->fontContext->font);
	free(c->fontContext);

	free(c->cam);
	free(c->faceCube);
	free(c);
	glfwTerminate();
}

void updateGame(Context *c) {
	/* Input handling */
	glfwPollEvents();

	/* Update data */
	mtx_lock(&c->gameMtx);
	glUseProgram(c->cubeShaderID);
	handle_input(c);
	update_camera(c, c->cubeShaderID);
	glUseProgram(0);
	mtx_unlock(&c->gameMtx);
}

void displayTextCall(Context *c, const char *description, f32 offsetHeight, u32 dataNumber, vec3 colorDescription, vec3 colorData) {
	char *dataString = ft_ultoa(dataNumber);
	textRender(c, description, TEXT_DESCRIBED_WIDTH_OFFSET, TEXT_HEIGHT_OFFSET_GET(offsetHeight), FPS_SCALE, colorDescription);
	textRender(c, (const char *)dataString, TEXT_DATA_WIDTH_OFFSET, TEXT_HEIGHT_OFFSET_GET(offsetHeight), FPS_SCALE, colorData);
	free(dataString);
}

void dataDisplay(Context *c) {
	displayTextCall(c, "FPS: ", 25.0f, fpsGet(), VEC3_YELLOW, VEC3_ORANGE);
	displayTextCall(c, "Chunk Rendered: ", 50.0f, c->chunkRenderedNb, VEC3_YELLOW, VEC3_GREEN);
	displayTextCall(c, "Chunk Loaded: ", 75.0f, c->chunkLoadedNb, VEC3_YELLOW, VEC3_RED);
	displayTextCall(c, "Face Rendered: ", 100.0f, c->faceRendered, VEC3_YELLOW, VEC3_GREEN);
}

void renderGame(Context *c, GLuint skyTexture) {
	/* Update render */
	renderChunksVBOhandling(c);
	/* Clear the color and depth buffer */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Render logic */
	displaySkybox(c->skyboxVAO, skyTexture, c->skyboxShaderID, c->cam->projection, c->cam->view);
	chunksRender(c, c->cubeShaderID);
	dataDisplay(c);
	/* glFlush forces the execution of all previous GL commands */
    glFlush();
	glfwSwapBuffers(c->win_ptr);
}


void mainLoopFpsLock(Context *c, GLuint skyTexture) {
    while (!glfwWindowShouldClose(c->win_ptr)) {
		updateGame(c);
		renderGame(c, skyTexture);	
    }
}

void mainLoopFpsUnlock(Context *c, GLuint skyTexture);

int main(void) {
    Context *context;

	if (!(context = contextInit())) {
		return (1);
	}

	mainLoopFpsUnlock(context, context->skyTexture);
	// mainLoopFpsLock(context, context->skyTexture);
    vox_destroy(context);
    return (0);
}


void mainLoopFpsUnlock(Context *c, GLuint skyTexture) {
    f64 lastTime = glfwGetTime();
    f64 timer = lastTime;
    f64 deltaTime = 0;
    f64 nowTime = 0;
    f64 updatePerSec = 60.0;

	/* Disable VSync to avoid fps locking */
	glfwSwapInterval(0);

    while (!glfwWindowShouldClose(c->win_ptr)) {
        nowTime = glfwGetTime();
        deltaTime += (nowTime - lastTime) * updatePerSec;
        lastTime = nowTime;
        
		/* Input update */
        while (deltaTime >= 1.0) {
            updateGame(c);
            deltaTime--;
        }
        /* Render */
        renderGame(c, skyTexture);
        /* Reset after one second */
        if (glfwGetTime() - timer > 1.0) {
            timer++;
        }
    }
}






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