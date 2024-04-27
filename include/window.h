#ifndef _HEADER_WINDOW_H
#define _HEADER_WINDOW_H 1

#include "vox.h"

/* openGL_glw_init.c */

GLFWwindow  *window_create(int width, int height, const char *title);
GLFWwindow  *init_openGL_context();
void        glfw_destroy(GLFWwindow *win);

#endif /* _HEADER_WINDOW_H */