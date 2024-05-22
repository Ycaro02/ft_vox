#include "../include/vox.h"			/* Main project header */
#include "../include/skybox.h"		/* skybox rendering */
#include "../include/chunks.h"		/* Chunks */
#include "../include/render_chunks.h"
#include "../include/perlin_noise.h"
#include "../include/thread_load.h"

void drawAllChunks(Context *c, GLuint VAO) {
	HashMap_it	it = hashmap_iterator(c->world->renderChunksMap);
	s8			next = 1;
	u32 		chunkRenderNb = 0;

	while ((next = hashmap_next(&it))) {
		drawAllCube(VAO, (RenderChunks *)it.value);
		chunkRenderNb++;
	}

	mtx_lock(&c->threadContext->mtx);
	ft_printf_fd(1, RESET_LINE""GREEN"Nb Chunk Rendered: %d,"RESET""ORANGE" Loaded: %d, "RESET""CYAN" To load: %d"RESET
		, chunkRenderNb, hashmap_size(c->world->chunksMap), hashmap_size(c->threadContext->chunksMapToLoad));
	mtx_unlock(&c->threadContext->mtx);
}

void chunksRender(Context *c, GLuint VAO, GLuint shader_id) {
    glLoadIdentity();
	glUseProgram(shader_id);
	drawAllChunks(c, VAO);
    glFlush();
}


void renderChunkCacheMapDestroy(HashMap *renderChunkCache, HashMap *renderChunks) {
	t_list *removeNodeList = NULL;
	t_list *removeDataList = NULL;
	HashMap_it it = hashmap_iterator(renderChunkCache);
	s8 next = TRUE;
	BlockPos *tmpChunkID = NULL;

	while ((next = hashmap_next(&it))) {
		BlockPos chunkID = ((RenderChunks *)it.value)->chunkID;
		tmpChunkID = malloc(sizeof(BlockPos));
		ft_memcpy(tmpChunkID, &chunkID, sizeof(BlockPos));
		if (chunksIsRenderer(renderChunks, chunkID)) {
			ft_lstadd_front(&removeNodeList, ft_lstnew(tmpChunkID));
		} else {
			ft_lstadd_front(&removeDataList, ft_lstnew(tmpChunkID));
		}
	}

	for (t_list *current = removeNodeList; current; current = current->next) {
		hashmap_remove_entry(renderChunkCache, *(BlockPos *)current->content, HASHMAP_FREE_NODE);
	}

	for (t_list *current = removeDataList; current; current = current->next) {
		hashmap_remove_entry(renderChunkCache, *(BlockPos *)current->content, HASHMAP_FREE_DATA);
	}


	ft_lstclear(&removeNodeList, free);
	ft_lstclear(&removeDataList, free);

	hashmap_destroy(renderChunkCache);
}

void vox_destroy(Context *c) {
	s32 status = 0;
	mtx_lock(&c->threadContext->mtx);
	c->isPlaying = FALSE;
	mtx_unlock(&c->threadContext->mtx);
	
	thrd_join(c->threadContext->supervisor, &status);
	ft_printf_fd(1, CYAN"Supervisor thread joined with status %d\n"RESET, status);

	// threadWaitForWorker(c);
	hashmap_destroy(c->threadContext->chunksMapToLoad);
	free(c->threadContext->workers);

	mtx_destroy(&c->threadContext->mtx);

	// hashmap_destroy(c->world->renderChunksCacheMap);
	renderChunkCacheMapDestroy(c->world->renderChunksCacheMap, c->world->renderChunksMap);
	hashmap_destroy(c->world->renderChunksMap);
	hashmap_destroy(c->world->chunksMap);

	// free(c->threadContext->workers);
	free(c->threadContext);
	free(c->world);
	free(c->cube.vertex);
	for (u32 i = 0; i < PERLIN_NOISE_HEIGHT; ++i) {
		free(c->perlin2D[i]);
	}
	free(c->perlin2D);
    // free(c->perlinNoise);
	glfwTerminate();
}

void main_loop(Context *c, GLuint vao, GLuint skyTexture) {
    while (!glfwWindowShouldClose(c->win_ptr)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		/* Input handling */
		glfwPollEvents();
        handle_input(c);
		
		/* Update data */
		update_camera(c, c->cubeShaderID);
		chunksViewHandling(c, c->world->renderChunksMap);

		/* Render logic */
        displaySkybox(c->skyboxVAO, skyTexture, c->skyboxShaderID, c->cam.projection, c->cam.view);
        chunksRender(c, vao, c->cubeShaderID);

		glfwSwapBuffers(c->win_ptr);
        // display_fps();
    }
}


u8 *perlinNoiseGeneration(unsigned int seed) {
	return (perlinImageGet(seed, PERLIN_NOISE_HEIGHT, PERLIN_NOISE_WIDTH, PERLIN_OCTAVE, PERLIN_PERSISTENCE, PERLIN_LACUNARITY));
}


u8 **array1DTo2D(u8 *array, u32 height, u32 width) {
	u8 **perlin2D = ft_calloc(height, sizeof(u8 *));
	for (u32 i = 0; i < height; ++i) {
		perlin2D[i] = ft_calloc(width, sizeof(u8));
		for (u32 j = 0; j < width; ++j) {
			perlin2D[i][j] = array[i * width + j];
		}
	}
	return (perlin2D);
}

int main() {
    Context context;
    GLFWwindow* window;

    window = init_openGL_context();

	ft_bzero(&context, sizeof(Context));
	context.isPlaying = TRUE;
    context.win_ptr = window;

	if (!(context.world = ft_calloc(sizeof(World), 1))) {
		return (1);
	} else if (!(context.world->chunksMap = hashmap_init(HASHMAP_SIZE_100, chunksMapFree))) {
		return (1);
	} else if (!(context.world->renderChunksCacheMap = hashmap_init(HASHMAP_SIZE_100, renderChunksMapFree))) {
		return (1);
	}

	u8 *perlin1D = perlinNoiseGeneration(42); /* seed 42 */
	if (!perlin1D) {
		ft_printf_fd(1, "Error: perlinNoise error\n");
		return (1);
	}
	/* Transform 1D array to 2D array */
	context.perlin2D = array1DTo2D(perlin1D, PERLIN_NOISE_HEIGHT, PERLIN_NOISE_WIDTH);
	free(perlin1D);

	/* init context camera */
	context.cam = create_camera(CAM_FOV, CAM_ASPECT_RATIO(SCREEN_WIDTH, SCREEN_HEIGHT), CAM_NEAR, CAM_FAR);
    glm_mat4_identity(context.cube.rotation);
	// display_camera_value(&context);


	/* Init chunks */
	if (!threadSupervisorInit(&context)) {
		ft_printf_fd(2, "Error: threadSupervisorInit failed\n");
		return (1);
	}
	// chunksLoadArround(&context, 10);
	GLuint cubeVAO = setupCubeVAO(&context.cube);
	context.world->renderChunksMap = chunksToRenderChunks(&context, context.world->chunksMap);

	/* Init skybox */
	context.skyboxVAO = skyboxInit();
	context.skyboxShaderID = load_shader(SKY_VERTEX_SHADER, SKY_FRAGMENT_SHADER);
	GLuint skyTexture = load_cubemap(TEXTURE_SKY_PATH, 1024, 1024);
	set_shader_texture(context.skyboxShaderID, skyTexture, GL_TEXTURE_CUBE_MAP, "texture1");

	/* Init cube */
	context.cubeShaderID = load_shader(CUBE_VERTEX_SHADER, CUBE_FRAGMENT_SHADER);
    GLuint textureAtlas = load_texture_atlas(TEXTURE_ATLAS_PATH, 16, 16);
	set_shader_texture(context.cubeShaderID, textureAtlas, GL_TEXTURE_3D, "textureAtlas");

	/* Disable VSync to avoid fps locking */
	// glfwSwapInterval(0);

	main_loop(&context, cubeVAO, skyTexture);

    vox_destroy(&context);
    return (0);
}
