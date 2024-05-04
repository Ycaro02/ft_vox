#include "../include/vox.h"			/* Main project header */
#include "../include/skybox.h"		/* skybox rendering */

void renderScene(t_context *c, GLuint vao, GLuint shader_id) {
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
	glUseProgram(shader_id);
    drawAllCube(vao, c->renderBlock);
    glFlush();
}


void vox_destroy(t_context *c, GLuint *atlas)
{
    hashmap_destroy(c->chunks->sub_chunks[0].block_map);
    free(atlas);
    free(c->chunks);
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
		ft_printf_fd(1, "\r\033[K"ORANGE"%f ms/frame, %d FPS"RESET, (1000.0 / (double)nbFrames), nbFrames);
		nbFrames = 0;
		lastTime += 1.0;
	}
}

FT_INLINE void main_loop(t_context *context, GLuint vao, GLuint skyTexture) {

    while (!glfwWindowShouldClose(context->win_ptr)) {
        update_camera(context, context->cubeShaderID);
    
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw skybox first
        displaySkybox(context->skyboxVAO, skyTexture, context->skyboxShaderID, context->cam.projection, context->cam.view);

        // glUseProgram(context->cubeShaderID);
        renderScene(context, vao, context->cubeShaderID);

      
	    glfwSwapBuffers(context->win_ptr);

        glfwPollEvents();
        handle_input(context);
        display_fps();
    }

}

int main() {
    t_context context;
    GLFWwindow* window;

    window = init_openGL_context();
    context.win_ptr = window;

    context.chunks = ft_calloc(sizeof(t_chunks), TEST_CHUNK_MAX);
    if (!context.chunks) {
        return (1);
    }

	/* init context camera */
	context.cam = create_camera(45.0f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
    glm_mat4_identity(context.cube.rotation);

	fillChunks(&context);
	GLuint vao = setupCubeVAO(&context, &context.cube);

	/* Init skybox */
	context.skyboxVAO = skyboxInit();
	context.skyboxShaderID = load_shader(SKY_VERTEX_SHADER, SKY_FRAGMENT_SHADER);
	GLuint skyTexture = load_cubemap(TEXTURE_SKY_PATH, 1024, 1024, (vec3_u8){0, 0, 0}); /* black */
	set_shader_texture(context.skyboxShaderID, &skyTexture, 0, GL_TEXTURE_CUBE_MAP);

	/* Init cube */
	context.cubeShaderID = load_shader(CUBE_VERTEX_SHADER, CUBE_FRAGMENT_SHADER);
    GLuint *textureAtlas = load_texture_atlas(TEXTURE_ATLAS_PATH, 16, 16, (vec3_u8){255, 0, 255}); /* PINK */
	set_shader_texture(context.cubeShaderID, textureAtlas, ATLAS_STONE, GL_TEXTURE_2D);

	/* Disable VSync to avoid fps locking */
	// glfwSwapInterval(0);

	main_loop(&context, vao, skyTexture);

    vox_destroy(&context, textureAtlas);
    return 0;
}
