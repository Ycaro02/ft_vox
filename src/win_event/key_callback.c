#include "../../include/vox.h"

/* Escapte Key : ESC */
void act_escape(t_context *c) {
    glfwSetWindowShouldClose(c->win_ptr, GL_TRUE);
}

/* Zoom : W */
void act_zoom(t_context *c) {
    move_camera_forward(&c->cam, 0.5f);
}

/* Unzoom : S */
void act_unzoom(t_context *c) {
    move_camera_backward(&c->cam, 0.5f);
}

/* Unzoom: D */
void act_move_right(t_context *c) {
	straf_camera(&c->cam, 0.5f, DIR_RIGHT);
}

/* Unzoom: A */
void act_move_left(t_context *c) {
	straf_camera(&c->cam, 0.5f, DIR_LEFT);
}

/* Rotate camera left : LEFT */
void act_rotate_camera_left(t_context *c) {
	rotate_camera(&c->cam, CAM_MOVE_HORIZONTAL, VEC3_ROTATEY);
}

/* Rotate camera right: RIGHT */
void act_rotate_camera_right(t_context *c) {
    rotate_camera(&c->cam, -CAM_MOVE_HORIZONTAL, VEC3_ROTATEY);
}

/* Rotate camera top UP */
void act_rotate_camera_top(t_context *c) {
	rotateTopBot(&c->cam, -CAM_MOVE_HORIZONTAL);
}

/* Rotate camera down : DOWN */
void act_rotate_camera_down(t_context *c) {
	rotateTopBot(&c->cam, CAM_MOVE_HORIZONTAL);
}

/* Up camera : SPACE */
void act_up_camera(t_context *c) {
    move_camera_up(&c->cam, CAM_UP_DOWN);
}

/* Down camera : Q */
void act_down_camera(t_context *c) {
    move_camera_up(&c->cam, -CAM_UP_DOWN);
}

/* Reset cam : ENTER */
void act_reset_camera(t_context *c) {
    reset_camera(c);
}

/* Display cam data : C */
void act_display_camera_value(t_context *c) {
    ft_printf_fd(1, CYAN"\nPos: %f, %f, %f\n"RESET, c->cam.position[0], c->cam.position[1],c->cam.position[2]);
	ft_printf_fd(1, PINK"Chunk Pos %d, %d, %d\n", c->cam.chunkPos[0], c->cam.chunkPos[1], c->cam.chunkPos[2]);
	// display_camera_value(&c->cam);
}

/* Change polygon mode : P */
void act_change_polygon_mode(t_context *c) {
	/* To store in context structure */
    static u8 fill_mode = 1;
	
	(void)c;
    fill_mode = !fill_mode;
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE + fill_mode);
}

/* Rotate object left : LEFT */
// void act_rotate_object_left(t_context *c) {
// 	rotate_object_around_center(&c->cube, VEC3_ROTATEX, ROTATE_ANGLE, c->cubeShaderID);
// }

// /* Rotate object right : RIGHT */
// void act_rotate_object_right(t_context *c) {
// 	rotate_object_around_center(&c->cube, VEC3_ROTATEX, -ROTATE_ANGLE, c->cubeShaderID);
// }

// /* Rotate object up : UP */
// void act_rotate_object_up(t_context *c) {
// 	rotate_object_around_center(&c->cube, VEC3_ROTATEY, ROTATE_ANGLE, c->cubeShaderID);
// }

// /* Rotate object down : DOWN */
// void act_rotate_object_down(t_context *c) {
// 	rotate_object_around_center(&c->cube, VEC3_ROTATEY, -ROTATE_ANGLE, c->cubeShaderID);
// }

// /* Rotate object Z up : PAGE UP*/
// void act_rotate_object_z_up(t_context *c) {
// 	rotate_object_around_center(&c->cube, VEC3_ROTATEZ, ROTATE_ANGLE, c->cubeShaderID);
// }


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
		{GLFW_KEY_P, act_change_polygon_mode, SINGLE_PRESS},
		{GLFW_KEY_W, act_zoom, REPEAT},
		{GLFW_KEY_S, act_unzoom, REPEAT},
		{GLFW_KEY_D, act_move_right, REPEAT},
		{GLFW_KEY_A, act_move_left, REPEAT},
		{GLFW_KEY_SPACE, act_up_camera, REPEAT},
		{GLFW_KEY_Q, act_down_camera, REPEAT},
		{GLFW_KEY_LEFT, act_rotate_camera_left, REPEAT},
		{GLFW_KEY_RIGHT, act_rotate_camera_right, REPEAT},
		{GLFW_KEY_UP, act_rotate_camera_top, REPEAT},
		{GLFW_KEY_DOWN, act_rotate_camera_down, REPEAT},
		{GLFW_KEY_ENTER, act_reset_camera, SINGLE_PRESS},
		{GLFW_KEY_C, act_display_camera_value, SINGLE_PRESS},
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