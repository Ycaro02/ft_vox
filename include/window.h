#ifndef HEADER_WINDOW_GLFW
#define HEADER_WINDOW_GLFW 1

// #include "vox.h"
#include "typedef_struct.h"

/* openGL_glw_init.c */

GLFWwindow  *window_create(int width, int height, const char *title);
GLFWwindow *init_openGL_context(s32 *height, s32 *width, const char *title);
void        glfw_destroy(GLFWwindow *win);

FT_INLINE  u32 fpsGet() {
	static f64 lastTime = 0.0f; 
	static u32 nbFrames = 0;
	static u32 saveFrames = 0;

	if (lastTime == 0.0f) {
		lastTime = glfwGetTime();
	}
	f64 currentTime = glfwGetTime();
	nbFrames++;
	if (currentTime - lastTime >= 1.0) {
		// ft_printf_fd(1, RESET_LINE""ORANGE"%f ms/frame, %d FPS"RESET, (1000.0 / (f64)nbFrames), nbFrames);
		saveFrames = nbFrames;
		nbFrames = 0;
		lastTime += 1.0;
	}
	return (saveFrames);
}


#endif /* HEADER_WINDOW_GLFW */