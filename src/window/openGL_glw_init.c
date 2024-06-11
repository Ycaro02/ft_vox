#include "../../include/vox.h"

GLFWwindow *window_create(int width, int height, const char *title)
{
	GLFWwindow *win;

    if (!glfwInit()) {
		ft_printf_fd(2, "Error: Failed to initialize GLFW\n");
        return (NULL);
	}



	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	win = glfwCreateWindow(width, height, title, monitor, NULL);
	// win = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!win) {
		ft_printf_fd(2, "Error: Failed to create GLFW window\n");
		glfwTerminate();
        return (NULL);
    }



	/* Init openGL context on current thread */
    glfwMakeContextCurrent(win);
    return (win);
}

/**
 * @brief Initialize the openGL context
 * @return GLFWwindow* return the window struct pointer
*/
GLFWwindow *init_openGL_context() 
{
    GLFWwindow *win = NULL;
	int version = 0;
	
	win = window_create(SCREEN_WIDTH, SCREEN_HEIGHT, "VOX");
	if (!win) {
		return (NULL);
	}

	/* 
	 * Load all openGL function pointer with glad
	 * gladLoaderLoadGLLoader returns the version of glad 0 on error
	*/

 	if (!(version = gladLoaderLoadGL())) {
        ft_printf_fd(2, "Error: Failed to initialize Glad\n");
        glfwDestroyWindow(win);
        glfwTerminate();
        return (NULL);
    }

	/* Enable 8x antialiasing */
    glfwWindowHint(GLFW_SAMPLES, 8);

	/* Enable multisampling for antialiasing */
    glEnable(GL_MULTISAMPLE);
	
	/* Enable depth testing */
	glEnable(GL_DEPTH_TEST);

	/* Enable face culling */
	// glEnable(GL_CULL_FACE);

	/* Enable blending, make water transparent but we need to do occlusion culling between chunks */
    glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* Set the viewport */
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	
	const GLubyte* renderer = glGetString(GL_RENDERER);
	ft_printf_fd(1, CYAN"Renderer device: %s\n"RESET, renderer);

	/* Set the clear color */
	// glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    return (win);
}

/**
 * @brief Destroy glfw and free the model
 * @param win window struct
 * @param model model struct
*/
void glfw_destroy(GLFWwindow *win)
{

	ft_printf_fd(2, RED"Destroying glfw\n"RESET);
	glBindVertexArray(0);
	// glDeleteVertexArrays(1, &model->vao);
	// glDeleteProgram(model->shader_id);
	gladLoaderUnloadGL();
	glfwDestroyWindow(win);
	glfwTerminate();
	// free_obj_model(model);
}