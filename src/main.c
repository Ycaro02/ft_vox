#include "../include/vox.h"			/* Main project header */
#include "../include/skybox.h"		/* skybox rendering */
#include "../include/chunks.h"		/* Chunks */
#include "../include/render_chunks.h"
#include "../include/perlin_noise.h"
#include "../include/thread_load.h"

void drawAllChunks(GLuint VAO, HashMap *renderChunksMap) {
	HashMap_it	it = hashmap_iterator(renderChunksMap);
	s8			next = 1;

	while ((next = hashmap_next(&it))) {
		drawAllCube(VAO, (RenderChunks *)it.value);
	}
}

void chunksRender(Context *c, GLuint VAO, GLuint shader_id, HashMap *renderChunksMap) {
	(void)c;
    glLoadIdentity();
	glUseProgram(shader_id);
	drawAllChunks(VAO, renderChunksMap);
    glFlush();
}


void vox_destroy(Context *c, HashMap *renderChunksMap) {
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

	hashmap_destroy(renderChunksMap);
	hashmap_destroy(c->world->chunksMap);

	// free(c->threadContext->workers);
	free(c->threadContext);
	free(c->world);
	free(c->cube.vertex);
    free(c->perlinNoise);
	glfwTerminate();
}

FT_INLINE void main_loop(Context *context, GLuint vao, GLuint skyTexture, HashMap *renderChunksMap) {
    while (!glfwWindowShouldClose(context->win_ptr)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		/* Input handling */
		glfwPollEvents();
        handle_input(context);
		
		/* Update data */
		update_camera(context, context->cubeShaderID);
		chunksViewHandling(context, renderChunksMap);

		/* Render logic */
        displaySkybox(context->skyboxVAO, skyTexture, context->skyboxShaderID, context->cam.projection, context->cam.view);
        chunksRender(context, vao, context->cubeShaderID, renderChunksMap);

		glfwSwapBuffers(context->win_ptr);
        // display_fps();
    }
}


u8 *perlinNoiseGeneration(unsigned int seed) {
	return (perlinImageGet(seed, PERLIN_NOISE_HEIGHT, PERLIN_NOISE_WIDTH, PERLIN_OCTAVE, PERLIN_PERSISTENCE, PERLIN_LACUNARITY));
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
	}

	context.perlinNoise = perlinNoiseGeneration(42); /* seed 42 */
	if (!context.perlinNoise) {
		ft_printf_fd(1, "Error: perlinNoise error\n");
		return (1);
	}

	/* init context camera */
	context.cam = create_camera(80.0f, (float)(SCREEN_WIDTH / SCREEN_HEIGHT), 0.1f, 100.0f);
    glm_mat4_identity(context.cube.rotation);
	// display_camera_value(&context);


	/* Init chunks */
	if (!threadSupervisorInit(&context)) {
		ft_printf_fd(2, "Error: threadSupervisorInit failed\n");
		return (1);
	}
	// chunksLoadArround(&context, 10);
	GLuint cubeVAO = setupCubeVAO(&context.cube);
	HashMap *renderChunksMap = chunksToRenderChunks(&context, context.world->chunksMap);

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

	main_loop(&context, cubeVAO, skyTexture, renderChunksMap);

    vox_destroy(&context, renderChunksMap);
    return (0);
}
