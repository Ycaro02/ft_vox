#include "../include/vox.h"			/* Main project header */

#include "../include/vox.h"     /* Main project header */

void drawCube(GLuint vao) {
    // Draw the cube using triangle fans
	glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 8);
	glBindVertexArray(0);
}

void renderScene(GLuint vao, GLuint shader_id) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
	glUseProgram(shader_id);
    drawCube(vao);
    glFlush();
}

GLuint setupCubeVAO() {
    // Define vertices for the cube
    static const GLfloat vertices[] = {
        // Vertices of the cube faces
        -0.5f, -0.5f, 0.5f,
        0.5f, -0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, 0.5f, -0.5f,
        -0.5f, 0.5f, -0.5f
    };

    GLuint VAO, VBO;

    // Generate vertex array object (VAO)
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Generate vertex buffer object (VBO)
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Specify vertex attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Unbind VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return (VAO);
}

int main() {
    t_context context;
    GLFWwindow* window;
	t_render render;

	ft_bzero(&render, sizeof(t_render));

    // Initialize the library
    window = init_openGL_context();

    context.win_ptr = window;
	GLuint vao = setupCubeVAO();
	GLuint shader_id = load_shader(&render);
    // Set the callback function for rendering
    while (!glfwWindowShouldClose(window)) {
        // Render here
        renderScene(vao, shader_id);

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
