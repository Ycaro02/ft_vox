#include "../include/vox.h"			/* Main project header */

void renderScene(GLuint vao, GLuint shader_id) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
	glUseProgram(shader_id);
    drawCube(vao);
    glFlush();
}

int main() {
    t_context context;
    GLFWwindow* window;
	t_render render;

	ft_bzero(&render, sizeof(t_render));

    // Initialize the library
    window = init_openGL_context();
    context.win_ptr = window;
	GLuint vao = setupCubeVAO(&context.cube);
	render.shader_id = load_shader(&render);
	context.shader_id = render.shader_id;
	context.cam = create_camera(45.0f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
    mat4_identity(context.cube.rotation);
	// Set the callback function for rendering
    while (!glfwWindowShouldClose(window)) {
        // Render here
		update_camera(&context, render.shader_id);
        renderScene(vao, render.shader_id);

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
        handle_input(&context);
    }

    // Terminate GLFW
    glfwTerminate();
    return 0;
}
