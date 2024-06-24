// #include <ft2build.h>
// #include FT_FREETYPE_H
#include "../include/text_render.h" /* text rendering, need to include this first  */
#include "../include/vox.h"			/* Main project header */
#include "../include/world.h"
#include "../include/skybox.h"		/* skybox rendering */
#include "../include/render_chunks.h"
#include "../rsc/perlin_noise/include/perlin_noise.h"
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

void localBlockDataUpdate(Context *c) {
	/*	
		Here when chunks mtx is lock we can get all wanted information
		Then we can access at chunk->noiseData.NOISE_FIELD[blockPos.x][blockPos.z]
		Copy all value in a local struct to display it to avoid possible datarace
	*/
	Chunks		*chunk = NULL;
	BlockPos	localBlockPos = {0};
	s32			chunkX = 0, chunkZ = 0;
	vec3		camPos = {0};

	mtx_lock(&c->gameMtx);
	chunkX = c->cam->chunkPos[0];
	chunkZ = c->cam->chunkPos[2];
	glm_vec3_copy(c->cam->position, camPos);
	mtx_unlock(&c->gameMtx);

	chunk = getChunkAt(c, chunkX, chunkZ);
	if (!chunk) {
		return ;
	}
	blockLocalPosFromCam(camPos, &localBlockPos);
	c->displayData.chunkX = chunkX;
	c->displayData.chunkZ = chunkZ;
	c->displayData.blockPos = localBlockPos;
	c->displayData.noiseData.valContinental = chunk->noiseData[localBlockPos.x][localBlockPos.z].valContinent;
	c->displayData.noiseData.valErosion = chunk->noiseData[localBlockPos.x][localBlockPos.z].valErosion;
	c->displayData.noiseData.valPeaksValley = chunk->noiseData[localBlockPos.x][localBlockPos.z].valPeaksValley;
	c->displayData.noiseData.valCombined = chunk->noiseData[localBlockPos.x][localBlockPos.z].valCombined;
	c->displayData.noiseData.valHumidity = chunk->noiseData[localBlockPos.x][localBlockPos.z].valHumidity;
	c->displayData.noiseData.valTemperature = chunk->noiseData[localBlockPos.x][localBlockPos.z].valTemperature;
	c->displayData.posNoise[0] = chunk->noiseData[localBlockPos.x][localBlockPos.z].x0;
	c->displayData.posNoise[1] = chunk->noiseData[localBlockPos.x][localBlockPos.z].z0;
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

	localBlockDataUpdate(c);


	c->displayData.chunkLoadedNb = hashmap_size(c->world->chunksMap);
	c->displayData.chunkToLoadInQueue = hashmap_size(c->threadContext->chunksMapToLoad);
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

	/* noise free */
	free_incomplete_array((void **)c->world->noise.continental, PERLIN_NOISE_HEIGHT);
	free_incomplete_array((void **)c->world->noise.erosion, PERLIN_NOISE_HEIGHT);
	free_incomplete_array((void **)c->world->noise.peaksValley, PERLIN_NOISE_HEIGHT);
	free_incomplete_array((void **)c->world->noise.humidity, PERLIN_NOISE_HEIGHT);
	free_incomplete_array((void **)c->world->noise.temperature, PERLIN_NOISE_HEIGHT);
	free_incomplete_array((void **)c->world->noise.cave, PERLIN_SNAKE_HEIGHT);


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

	free(c->world);
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

/**
 * @brief Display a float number with a description
*/
void displayFloatTextCall(Context *c, const char *description, f32 offsetHeight, f32 floatNumber, vec3 colorDescription, vec3 colorData) {
	t_sstring floatString = double_to_sstring(floatNumber, 3);
	textRender(c, description, TEXT_DESCRIBED_WIDTH_OFFSET, TEXT_HEIGHT_OFFSET_GET(offsetHeight), FPS_SCALE, colorDescription);
	textRender(c, floatString.data, TEXT_DATA_WIDTH_OFFSET, TEXT_HEIGHT_OFFSET_GET(offsetHeight), FPS_SCALE, colorData);
}

/**
 * @brief Display a unsigned 32 number with a description
*/
void displayUnsigned32TextCall(Context *c, const char *description, f32 offsetHeight, u32 dataNumber, vec3 colorDescription, vec3 colorData) {
	char *dataString = ft_ultoa(dataNumber);
	textRender(c, description, TEXT_DESCRIBED_WIDTH_OFFSET, TEXT_HEIGHT_OFFSET_GET(offsetHeight), FPS_SCALE, colorDescription);
	textRender(c, (const char *)dataString, TEXT_DATA_WIDTH_OFFSET, TEXT_HEIGHT_OFFSET_GET(offsetHeight), FPS_SCALE, colorData);
	free(dataString);
}

/**
 * @brief Display a signed long number with a description
*/
void displaySignedLongTextCall(Context *c, const char *description, f32 offsetHeight, s32 dataNumber, vec3 colorDescription, vec3 colorData) {
	char *dataString = ft_ltoa(dataNumber);
	textRender(c, description, TEXT_DESCRIBED_WIDTH_OFFSET, TEXT_HEIGHT_OFFSET_GET(offsetHeight), FPS_SCALE, colorDescription);
	textRender(c, (const char *)dataString, TEXT_DATA_WIDTH_OFFSET, TEXT_HEIGHT_OFFSET_GET(offsetHeight), FPS_SCALE, colorData);
	free(dataString);
}

/**
 * @brief Display a double axis position with a description
*/
void displayDoublePosition(Context* c, const char *description, float startY, long posX, long posZ, vec3 color) {
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "X %ld / Z %ld", posX, posZ);
    textRender(c, description, TEXT_DESCRIBED_WIDTH_OFFSET, TEXT_HEIGHT_OFFSET_GET(startY), FPS_SCALE, color);
    textRender(c, buffer, TEXT_DATA_WIDTH_OFFSET, TEXT_HEIGHT_OFFSET_GET(startY), FPS_SCALE, VEC3_PINK);
}

/**
 * @brief Display data on screen
*/
void dataDisplay(Context *c) {
	displayUnsigned32TextCall(c, "FPS: ", 25.0f, fpsGet(), VEC3_YELLOW, VEC3_ORANGE);
	displayUnsigned32TextCall(c, "Chunk Rendered: ", 50.0f, c->displayData.chunkRenderedNb, VEC3_YELLOW, VEC3_GREEN);
	displayUnsigned32TextCall(c, "Chunk Loaded: ", 75.0f, c->displayData.chunkLoadedNb, VEC3_YELLOW, VEC3_RED);
	displayUnsigned32TextCall(c, "Face Rendered: ", 100.0f, c->displayData.faceRendered, VEC3_YELLOW, VEC3_GREEN);
	/* Diplay chunk position */
	displayDoublePosition(c, "Chunk Pos: ", 125.0f, c->displayData.chunkX, c->displayData.chunkZ, VEC3_YELLOW);
	/* Display Block position */
	displayDoublePosition(c, "Block Pos: ", 150.0f, c->displayData.blockPos.x + (16 * c->displayData.chunkX)
		, c->displayData.blockPos.z + (16 * c->displayData.chunkZ), VEC3_YELLOW);
	displayFloatTextCall(c, "Val Continental: ", 175.0f, c->displayData.noiseData.valContinental, VEC3_YELLOW, VEC3_BLACK);
	displayFloatTextCall(c, "Val Erosion: ", 200.0f, c->displayData.noiseData.valErosion, VEC3_YELLOW, VEC3_BLACK);
	displayFloatTextCall(c, "Val PeaksValley: ", 225.0f, c->displayData.noiseData.valPeaksValley, VEC3_YELLOW, VEC3_BLACK);
	displayFloatTextCall(c, "Val Combined: ", 250.0f, c->displayData.noiseData.valCombined, VEC3_YELLOW, VEC3_BLACK);
	displayFloatTextCall(c, "Val Humidity: ", 275.0f, c->displayData.noiseData.valHumidity, VEC3_YELLOW, VEC3_BLACK);
	displayFloatTextCall(c, "Val Temperature: ", 300.0f, c->displayData.noiseData.valTemperature, VEC3_YELLOW, VEC3_BLACK);
	displayDoublePosition(c, "Noise Pos: ", 325.0f, c->displayData.posNoise[0], c->displayData.posNoise[1], VEC3_YELLOW);
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