#ifndef HEADER_WIN_EVENT_H
#define HEADER_WIN_EVENT_H

#include "typedef_struct.h"

/* Key repeat, key_action->repeat field */
#define SINGLE_PRESS	0				/* key action will be done only once */
#define REPEAT			1				/* key action will be done until key release */


/* Key action structure */
struct s_key_action {
    int key;					/* key code */
    void (*action)(Context *c);	/* action to do, function ptr void (t_obj_model *model)*/
	u8	repeat;					/* auto repeat until key release */
};

void handle_input(void *context);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

#endif /* HEADER_WIN_EVENT_H */
