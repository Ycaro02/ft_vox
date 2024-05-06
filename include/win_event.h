#ifndef HEADER_WIN_EVENT_H
#define HEADER_WIN_EVENT_H

/* Key repeat, key_action->repeat field */
#define SINGLE_PRESS	0				/* key action will be done only once */
#define REPEAT			1				/* key action will be done until key release */

/* Key action structure */
typedef struct s_key_action {
    int key;			/* key code */
    void (*action)();	/* action to do, function ptr void (t_obj_model *model)*/
	u8	repeat;			/* auto repeat until key release */
} KeyAction;

void handle_input(void *context);

#endif /* HEADER_WIN_EVENT_H */
