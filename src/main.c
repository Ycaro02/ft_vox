#include "../include/vox.h"			/* Main project header */

void renderScene(t_context *c, GLuint vao, GLuint shader_id) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

FT_INLINE void main_loop(t_context *context, GLuint vao, t_render *render) {

    while (!glfwWindowShouldClose(context->win_ptr)) {
		update_camera(context, render->shader_id);
        renderScene(context, vao, render->shader_id);

        glfwSwapBuffers(context->win_ptr);

        glfwPollEvents();
        handle_input(context);
		display_fps();
    }

}

int main() {
    t_context context;
    GLFWwindow* window;
	t_render render;

	ft_bzero(&render, sizeof(t_render));

    window = init_openGL_context();
    context.win_ptr = window;

    context.chunks = ft_calloc(sizeof(t_chunks), TEST_CHUNK_MAX);
    if (!context.chunks) {
        return (1);
    }
    ft_printf_fd(1, "%u byte allocated\n", sizeof(t_chunks));

	fillChunks(&context);

	GLuint vao = setupCubeVAO(&context, &context.cube);
	render.shader_id = load_shader(&render);
	context.shader_id = render.shader_id;
	context.cam = create_camera(45.0f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
    glm_mat4_identity(context.cube.rotation);


	int width_texture = 4096 / 4;
	int height_texture = 3072 / 3;
	GLuint *skyAtlas = load_texture_atlas(TEXTURE_SKYBOX_PATH, height_texture, width_texture);
	(void)skyAtlas;


    GLuint *texture_atlas = load_texture_atlas(TEXTURE_ATLAS_PATH, 16, 16);
	set_shader_texture(&context, texture_atlas, ATLAS_STONE);



	/* Disable VSync to avoid fps locking */
	// glfwSwapInterval(0);

	main_loop(&context, vao, &render);

    vox_destroy(&context, texture_atlas);
    return 0;
}
