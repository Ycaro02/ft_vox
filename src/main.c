#include "../include/vox.h"
#include "../include/window.h"
#include "../include/win_event.h"

/**
 * @brief Main loop of the program
 * @param c context structure
*/
void main_loop(t_context *c) 
{
    while (!glfwWindowShouldClose(c->win_ptr)) {
        /* clear gl render context*/
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		/* bind vertex array */
		// glBindVertexArray(c->vao);
		// glDrawElements(GL_TRIANGLES, (c->tri_size * 3), GL_UNSIGNED_INT, 0);

		/* unbind vertex array */
		// glBindVertexArray(0);
		/* swap buff to display */
		glfwSwapBuffers(c->win_ptr);
        /* Event handling */
        glfwPollEvents();
		handle_input(c);
    }
}

int main(int argc, char **argv)
{
	(void)argc, (void)argv;
	t_context c = {};
	ft_printf_fd(1, "Hi from main\n");
	c.win_ptr = init_openGL_context();
	main_loop(&c);
	return (0);
}
