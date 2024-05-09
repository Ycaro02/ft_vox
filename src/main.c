#include "../include/vox.h"			/* Main project header */
#include "../include/skybox.h"		/* skybox rendering */
#include "../include/render_chunks.h"
#include "../include/perlin_noise.h"

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


void vox_destroy(Context *c)
{
	hashmap_destroy(c->world->chunksMap);
	free(c->world);
	free(c->cube.vertex);
    free(c->perlinNoise);
	glfwTerminate();
}

FT_INLINE void display_fps() {
	static double lastTime = 0.0f; 
	static int nbFrames = 0;

	if (lastTime == 0.0f) {
		lastTime = glfwGetTime();
	}
	double currentTime = glfwGetTime();
	nbFrames++;
	if (currentTime - lastTime >= 1.0) {
		ft_printf_fd(1, RESET_LINE""ORANGE"%f ms/frame, %d FPS"RESET, (1000.0 / (double)nbFrames), nbFrames);
		nbFrames = 0;
		lastTime += 1.0;
	}
}

FT_INLINE void main_loop(Context *context, GLuint vao, GLuint skyTexture, HashMap *renderChunksMap) {


    while (!glfwWindowShouldClose(context->win_ptr)) {
    
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw skybox first
        displaySkybox(context->skyboxVAO, skyTexture, context->skyboxShaderID, context->cam.projection, context->cam.view);
        chunksRender(context, vao, context->cubeShaderID, renderChunksMap);

	    glfwSwapBuffers(context->win_ptr);
        glfwPollEvents();
        handle_input(context);
        update_camera(context, context->cubeShaderID);
        display_fps();
    }
}


/* Basic function you can provide to hashmap_init */
void chunksMapFree(void *entry) {
	HashMap_entry *e = (HashMap_entry *)entry;
	if (e->value) {
		Chunks *chunks = (Chunks *)e->value;
		for (u32 i = 0; chunks->sub_chunks[i].block_map ; ++i) {
			hashmap_destroy(chunks->sub_chunks[i].block_map);
		}
		// hashmap_destroy(chunks->sub_chunks[1].block_map);
		free(e->value); /* free the value (allocaated ptr) */
	}
	free(e); /* free the entry t_list node */
}

u8 *perlinNoiseGeneration(unsigned int seed) {
	return (perlinImageGet(seed, PERLIN_NOISE_HEIGHT, PERLIN_NOISE_WIDTH, 4, 2.0, 2.0));
}

int main() {
    Context context;
    GLFWwindow* window;

	ft_bzero(&context, sizeof(Context));
    window = init_openGL_context();
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

	chunksLoadArround(&context, 1);
	GLuint cubeVAO = setupCubeVAO(&context, &context.cube);
	HashMap *renderChunksMap = chunksToRenderChunks(&context, context.world->chunksMap);

	/* Init skybox */
	context.skyboxVAO = skyboxInit();
	context.skyboxShaderID = load_shader(SKY_VERTEX_SHADER, SKY_FRAGMENT_SHADER);
	GLuint skyTexture = load_cubemap(TEXTURE_SKY_PATH, 1024, 1024, (vec3_u8){0, 0, 0}); /* black */
	set_shader_texture(context.skyboxShaderID, skyTexture, GL_TEXTURE_CUBE_MAP, "texture1");

	/* Init cube */
	context.cubeShaderID = load_shader(CUBE_VERTEX_SHADER, CUBE_FRAGMENT_SHADER);
    // GLuint *textureAtlas = load_texture_atlas(TEXTURE_ATLAS_PATH, 16, 16, (vec3_u8){255, 0, 255}); /* PINK */
    GLuint textureAtlas = load_texture_atlas(TEXTURE_ATLAS_PATH, 16, 16, (vec3_u8){255, 0, 255}); /* PINK */
	set_shader_texture(context.cubeShaderID, textureAtlas, GL_TEXTURE_3D, "textureAtlas");

	/* Disable VSync to avoid fps locking */
	// glfwSwapInterval(0);

	main_loop(&context, cubeVAO, skyTexture, renderChunksMap);

	// ft_lstclear(&renderChunksList, free);
	hashmap_destroy(renderChunksMap);
    vox_destroy(&context);
    return 0;
}
