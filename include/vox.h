#ifndef VOX_HEADER_H
#define VOX_HEADER_H

#include <math.h>

#include "glad/gl.h" /* include glad header before glfw3 */
#include "glfw3/glfw3.h"
#include "../libft/libft.h"
#include "../libft/parse_flag/parse_flag.h"
#include "../libft/stack_string/stack_string.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

/* Key repeat, key_action->repeat field */
#define SINGLE_PRESS	0				/* key action will be done only once */
#define REPEAT			1				/* key action will be done until key release */

typedef struct s_context {
    GLFWwindow *win_ptr;
} t_context;

/* Key action structure */
typedef struct s_key_action {
    int key;							/* key code */
    void (*action)(t_context *c);	/* action to do, function ptr void (t_obj_model *model)*/
	u8	repeat;							/* auto repeat until key release */
} t_key_action;

#endif /* VOX_HEADER_H */