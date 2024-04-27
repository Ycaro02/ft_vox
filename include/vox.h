#ifndef VOX_HEADER_H
#define VOX_HEADER_H

#include <math.h>					/* LIBC Math functions */
#include "glad/gl.h"				/* Glad functions (Include glad header BEFORE glfw3) */
#include "glfw3/glfw3.h"			/* GLFW functions */
#include "libft/libft.h"			/* Libft functions */
#include "libft/parse_flag.h"		/* Parse/handle flag functions */
#include "libft/stack_string.h"		/* Stack string handle functions */
#include "window.h"					/* Window related functions */
#include "win_event.h"				/* Window event handling functions */

#define SCREEN_WIDTH 800			/* Screen width */
#define SCREEN_HEIGHT 600			/* Screen height */

/* Context structure */
typedef struct s_context {
    GLFWwindow *win_ptr;			/* Window pointer */
} t_context;

#endif /* VOX_HEADER_H */