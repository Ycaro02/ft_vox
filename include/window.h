#ifndef _HEADER_WINDOW_H
#define _HEADER_WINDOW_H 1

#include "vox.h"

/* openGL_glw_init.c */

GLFWwindow  *window_create(int width, int height, const char *title);
GLFWwindow  *init_openGL_context();
void        glfw_destroy(GLFWwindow *win);

FT_INLINE  void display_fps() {
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


#endif /* _HEADER_WINDOW_H */