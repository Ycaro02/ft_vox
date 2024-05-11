#include "../../include/world.h"
#include "../../include/chunks.h"

/* Escapte Key : ESC */
void act_escape(Context *c) {
    glfwSetWindowShouldClose(c->win_ptr, GL_TRUE);
}

/* Zoom : W */
void act_zoom(Context *c) {
    move_camera_forward(&c->cam, 0.2f);
}

/* Unzoom : S */
void act_unzoom(Context *c) {
    move_camera_backward(&c->cam, 0.2f);
}

/* Unzoom: D */
void act_move_right(Context *c) {
	straf_camera(&c->cam, 0.1f, DIR_RIGHT);
}

/* Unzoom: A */
void act_move_left(Context *c) {
	straf_camera(&c->cam, 0.1f, DIR_LEFT);
}

/* Rotate camera left : LEFT */
void act_rotate_camera_left(Context *c) {
	rotate_camera(&c->cam, 1.0f, VEC3_ROTATEY);
}

/* Rotate camera right: RIGHT */
void act_rotate_camera_right(Context *c) {
    rotate_camera(&c->cam, -1.0f, VEC3_ROTATEY);
}

/* Rotate camera top UP */
void act_rotate_camera_top(Context *c) {
	rotateTopBot(&c->cam, -1.0f);
}

/* Rotate camera down : DOWN */
void act_rotate_camera_down(Context *c) {
	rotateTopBot(&c->cam, 1.0f);
}

/* Up camera : SPACE */
void act_up_camera(Context *c) {
    move_camera_up(&c->cam, CAM_UP_DOWN);
}

/* Down camera : Q */
void act_down_camera(Context *c) {
    move_camera_up(&c->cam, -CAM_UP_DOWN);
}

/* Reset cam : ENTER */
void act_reseCamera(Context *c) {
    reseCamera(c);
}

void testChunksExist(Context *c) {
	BlockPos pos = {0, c->cam.chunkPos[0], c->cam.chunkPos[2]};
	// ft_printf_fd(1, "Cam position: X|%f, Y:%f Z:|%f, \n", c->cam.position[0], c->cam.position[1], c->cam.position[2]);
	display_camera_value(&c->cam);
	ft_printf_fd(1, YELLOW"\nTest for chunk:"RESET" "ORANGE"X|%d| Z|%d|"RESET, pos.y, pos.z);
	Chunks *chunks = hashmap_get(c->world->chunksMap, pos);
	if (chunks) {
		ft_printf_fd(1, GREEN"Chunk exist\n"RESET);
	} else {
		ft_printf_fd(1, RED"Chunk not exist\n"RESET);
	}

	Camera *cam = &c->cam;
	f32 xDir = cam->viewVector[0];
	// f32 yDir = cam->viewVector[1];
	f32 zDir = cam->viewVector[2];

	char *NordSouth = "Unknown";
	if (xDir >= 0.0f) {
		NordSouth = CYAN"Nord"RESET;
	} else if (xDir < 0.0f) {
		NordSouth = RED"South"RESET;
	} 

	char *EstWest = "Unknown";
	if (zDir > 0.0f) {
		EstWest = ORANGE"East"RESET;
	} else if (zDir < 0.0f) {
		EstWest = PINK"West"RESET;
	}

	ft_printf_fd(1, PINK"View vec: X|%f|, Z|%f| "RESET":\n[%s] [%s]\n", xDir, zDir, NordSouth, EstWest);

}

/* Display cam data : C */
void act_display_camera_value(Context *c) {
    ft_printf_fd(1, CYAN"\nPos: %f, %f, %f\n"RESET, c->cam.position[0], c->cam.position[1],c->cam.position[2]);
	ft_printf_fd(1, PINK"Chunk X[%d], Z[%d], Y:|%d|\n", c->cam.chunkPos[0], c->cam.chunkPos[2], c->cam.chunkPos[1]);
	// display_camera_value(&c->cam);
}

/* Change polygon mode : P */
void act_change_polygon_mode(Context *c) {
	/* To store in context structure */
    static u8 fill_mode = 1;
	
	(void)c;
    fill_mode = !fill_mode;
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE + fill_mode);
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
	KeyAction	key_actions[] = {
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
		{GLFW_KEY_ENTER, act_reseCamera, SINGLE_PRESS},
		{GLFW_KEY_C, act_display_camera_value, SINGLE_PRESS},
		{GLFW_KEY_E, testChunksExist, SINGLE_PRESS},
	};
	u32 			max = (sizeof(key_actions) / sizeof(KeyAction));
	s32				state = GLFW_RELEASE;
 	Context 		*c = context;

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