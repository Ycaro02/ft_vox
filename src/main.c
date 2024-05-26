#include "../include/vox.h"			/* Main project header */
#include "../include/skybox.h"		/* skybox rendering */
#include "../include/chunks.h"		/* Chunks */
#include "../include/render_chunks.h"
#include "../include/perlin_noise.h"
#include "../include/thread_load.h"



void drawAllChunks(Context *c, GLuint VAO) {
	mtx_lock(&c->threadContext->mtx);


	HashMap_it	it = hashmap_iterator(c->world->renderChunksMap);
	s8			next = 1;
	u32 		chunkRenderNb = 0;

	while ((next = hashmap_next(&it))) {
		drawAllCube(VAO, (RenderChunks *)it.value);
		chunkRenderNb++;
	}

	ft_printf_fd(1, RESET_LINE""GREEN"Chunk Rendered: %d,"RESET""ORANGE" Loaded: %d, "RESET""CYAN" In loading: %d, "RESET""PINK" FPS: %d "RESET
		, chunkRenderNb, hashmap_size(c->world->chunksMap), hashmap_size(c->threadContext->chunksMapToLoad), fpsGet());


	mtx_unlock(&c->threadContext->mtx);
}

void chunksRender(Context *c, GLuint VAO, GLuint shader_id) {
    glLoadIdentity();
	glUseProgram(shader_id);
	drawAllChunks(c, VAO);
    glFlush();
}

void vox_destroy(Context *c) {
	s32 status = 0;
	mtx_lock(&c->threadContext->mtx);
	c->isPlaying = FALSE;
	mtx_unlock(&c->threadContext->mtx);
	
	thrd_join(c->threadContext->supervisor, &status);
	ft_printf_fd(1, PINK"\nSupervisor thread joined with status %d\n"RESET, status);

	hashmap_destroy(c->threadContext->chunksMapToLoad);
	free(c->threadContext->workers);

	mtx_destroy(&c->threadContext->mtx);


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



f32 normalizeU8Tof32(u8 value, u8 start1, u8 stop1, f32 start2, f32 stop2) {
    return start2 + (stop2 - start2) * ((value - start1) / (f32)(stop1 - start1));
}

f32 **array1DTo2D(u8 *array, u32 height, u32 width) {
	f32 **perlin2D = ft_calloc(height, sizeof(f32 *));
	for (u32 i = 0; i < height; ++i) {
		perlin2D[i] = ft_calloc(width, sizeof(f32));
		for (u32 j = 0; j < width; ++j) {
			perlin2D[i][j] = normalizeU8Tof32(array[i * width + j], 0, 255, -1.0f, 1.0f);
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
	} else if (!(context.world->chunksMap = hashmap_init(HASHMAP_SIZE_2000, chunksMapFree))) {
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

	GLuint cubeVAO = setupCubeVAO(&context.cube);
	/* Free only entry pointer cause renderChunks stored in chunks structure in chunksMap */
	context.world->renderChunksMap = hashmap_init(HASHMAP_SIZE_2000, hashmap_free_node_only);

	/* Init chunks */
	if (!threadSupervisorInit(&context)) {
		ft_printf_fd(2, "Error: threadSupervisorInit failed\n");
		return (1);
	}

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