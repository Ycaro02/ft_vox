#include "../../include/vox.h"

/* Escapte Key : ESC */
void act_escape(t_context *c) {
    glfwSetWindowShouldClose(c->win_ptr, GL_TRUE);
}

/**
 * @brief Set key callback for the window
 * @param window 
 * @param key keycode receive from glfw
 * @param scancode unused
 * @param action action receive from glfw	
 * @param mode unused
*/
void handle_input(void *context)
{
	/* To store in context structure */
	static u8		previous_state[GLFW_KEY_LAST] = {0};
	t_key_action	key_actions[] = {
		{GLFW_KEY_ESCAPE, act_escape, SINGLE_PRESS},
	};
	u32 			max = (sizeof(key_actions) / sizeof(t_key_action));
	s32				state = GLFW_RELEASE;
 	t_context 		*c = context;

	for (u32 i = 0; i < max; i++) {
        state = glfwGetKey(c->win_ptr, key_actions[i].key);

		if (!key_actions[i].repeat) { /* if not repeat key */ 
			if (state == GLFW_PRESS \
				&& previous_state[key_actions[i].key] != (u8)GLFW_PRESS) { /* key pressed and previous state not key_pressed */
				key_actions[i].action(c);
			}
		} else if (state == GLFW_PRESS) { /* If repeat and key pressed */
			key_actions[i].action(c);
		}
        previous_state[key_actions[i].key] = state;
	}
}