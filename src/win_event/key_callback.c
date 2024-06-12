#include "../../include/world.h"
#include "../../include/chunks.h"
#include "../../include/perlin_noise.h"
#include "../../include/render_chunks.h"

/* Escapte Key : ESC */
void act_escape(Context *c) {
    glfwSetWindowShouldClose(c->win_ptr, GL_TRUE);
}

void act_change_cam_speed(Context *c) {
	c->cam.camSpeed = (c->cam.camSpeed == CAM_BASE_SPEED) ? (CAM_HIGHT_SPEED) : CAM_BASE_SPEED;
}

void act_autoMoveTrigger(Context *c) {
	c->autoMove = (c->autoMove == FALSE) ? TRUE : FALSE;
}

void act_autoRotateTrigger(Context *c) {
	c->autoRotate = (c->autoRotate == FALSE) ? TRUE : FALSE;
}

/* Zoom : W */
void act_zoom(Context *c) {
    move_camera_forward(&c->cam, c->cam.camSpeed);
}

/* Unzoom : S */
void act_unzoom(Context *c) {
    move_camera_backward(&c->cam, c->cam.camSpeed);
}

/* Unzoom: D */
void act_move_right(Context *c) {
	straf_camera(&c->cam, c->cam.camSpeed, DIR_RIGHT);
}

/* Unzoom: A */
void act_move_left(Context *c) {
	straf_camera(&c->cam, c->cam.camSpeed, DIR_LEFT);
}

/* Rotate camera left : LEFT */
void act_rotate_camera_left(Context *c) {
	rotate_camera(&c->cam, 1.0f, VEC3_ROTATEY);
	// rotate_camera(&c->cam, 3.0f, VEC3_ROTATEY);
}

/* Rotate camera right: RIGHT */
void act_rotate_camera_right(Context *c) {
    rotate_camera(&c->cam, -1.0f, VEC3_ROTATEY);
    // rotate_camera(&c->cam, -3.0f, VEC3_ROTATEY);
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


void displayPerlinNoise(s32 blockX, s32 blockZ, PerlinData perlinData) {
	(void)blockX, (void)blockZ;
// 	ft_printf_fd(1, CYAN"----\nBlock Z:|%d|, Z:|%d| -> Given X:[%d] Z[%d]: \n"RESET, blockX, blockZ, perlinData.givenX, perlinData.givenZ);
// 	ft_printf_fd(1, PINK"Perlin Idx0 : X:[%d] Z:[%d]\n"RESET, perlinData.x0, perlinData.z0);
// 	ft_printf_fd(1, PINK"Perlin Idx1 : X:[%d] Z:[%d]\n"RESET, perlinData.x1, perlinData.z1);
// 	ft_printf_fd(1, "Val:|%f| -> ", perlinData.val);
// 	ft_printf_fd(1, "Add:|%f| -> ", perlinData.add);
	ft_printf_fd(1, "Normalise:|%d|\n----\n", perlinData.normalise);
}


void displayChunkData(Context *c, Chunks *chunk){
	RenderChunks *renderChunk = chunk->render;
	ft_printf_fd(1, "Chunk data:\n-----------------\n"RESET);
	if (renderChunk) {
		char *isRender = (chunksIsRenderer(c->world->renderChunksMap, renderChunk->chunkID)) ? GREEN"YES"RESET : RED"NO"RESET;
		ft_printf_fd(1, YELLOW"Is in render map == "RESET"%s\n", isRender);
		for (u32 i = 0; i < 6; ++i) {
			ft_printf_fd(1, YELLOW"Face:|%u| Count %u-> "RESET, i, renderChunk->faceCount[i]);
			ft_printf_fd(1, ORANGE"Face VBO %u, TypeVBO %u\n"RESET, renderChunk->faceVBO[i], renderChunk->faceTypeVBO[i]);
		}
		ft_printf_fd(1, GREEN"Last update %u\n"RESET, (suseconds_t)(renderChunk->lastUpdate - get_ms_time()));
	} else {
		ft_printf_fd(1, RED"Render chunk is NULL\n"RESET);
	}
	ft_printf_fd(1, CYAN"Chunk End :\n-----------------\n"RESET);

}


void cameraToBlockPosition(vec3 camPos, BlockPos *blockPos) {
	blockPos->y = 0;
	blockPos->x = (s32)floor(camPos[0] * 2.0f) % 16;
	blockPos->z = (s32)floor(camPos[2] * 2.0f) % 16;
	if (blockPos->x < 0) { blockPos->x *= -1; }
	if (blockPos->z < 0) { blockPos->z *= -1; }
}

// s8 cameraViewIsUndergroundNoCave(Context *c){
// 	// mtx_lock(&c->gameMtx);
// 	BlockPos 	chunkPos = {0, c->cam.chunkPos[0], c->cam.chunkPos[2]};
// 	vec3 		camPos = {c->cam.position[0], c->cam.position[1], c->cam.position[2]};
// 	// mtx_unlock(&c->gameMtx);
// 	mtx_lock(&c->threadContext->chunkMtx);
// 	Chunks *chunk = hashmap_get(c->world->chunksMap, chunkPos);
// 	s32 blockX = (((s32)floor(camPos[0] * 2.0)) % 16);
// 	s32 blockZ = (((s32)floor(camPos[2] * 2.0)) % 16);
// 	s32 CurrentMaxHeight = chunk->perlinVal[blockX][blockZ].normalise;
// 	if (CurrentMaxHeight > camPos[1]) {
// 		mtx_unlock(&c->threadContext->chunkMtx);
// 		return (FALSE);
// 	}
// 	mtx_unlock(&c->threadContext->chunkMtx);
// 	return (TRUE);
// }

void displayBlockPosition(Chunks *chunk, BlockPos blockPos) {
	ft_printf_fd(1, "Block Pos:\n-----------------\n"RESET);
	ft_printf_fd(1, ORANGE"World : X|%d|, Z|%d|\n"RESET, blockPos.x + 16 * chunk->x, blockPos.z + 16 * chunk->z);
	ft_printf_fd(1,   CYAN"Local : X|%d|, Z|%d|\n"RESET, blockPos.x, blockPos.z);
	ft_printf_fd(1, "Block end:\n-----------------\n"RESET);

}

void testChunksExist(Context *c) {
	BlockPos chunkPos = CHUNKS_MAP_ID_GET(c->cam.chunkPos[0], c->cam.chunkPos[2]);
	ft_printf_fd(1, GREEN"\nCam position: X|%f, Y:%f Z:|%f\n"RESET, c->cam.position[0], c->cam.position[1], c->cam.position[2]);
	// display_camera_value(c);
	ft_printf_fd(1, YELLOW"Test for chunk:"RESET" "ORANGE"X|%d| Z|%d|"RESET, chunkPos.y, chunkPos.z);
	Chunks *chunks = hashmap_get(c->world->chunksMap, chunkPos);
	if (chunks) {
		ft_printf_fd(1, GREEN" -> Chunk exist\n"RESET);
		BlockPos blockPos = {0};
		cameraToBlockPosition(c->cam.position, &blockPos);
		Chunks *chunk = hashmap_get(c->world->chunksMap, chunkPos);
		Block *block = getBlockAt(chunk, blockPos.x, blockPos.y, blockPos.z);
		displayBlockPosition(chunk, blockPos);
		ft_printf_fd(1, CYAN"Perlin Height\n");
		displayPerlinNoise(blockPos.x,blockPos.z,chunk->perlinVal[blockPos.x][blockPos.z]);

		// mtx_lock(&c->vboToCreateMtx);
		// displayChunkData(c, chunk);
		// mtx_unlock(&c->vboToCreateMtx);
		/* TO restore now u8 val */
		// ft_printf_fd(1, RED"Perlin cave\n");
		// ft_printf_fd(1, "Val: %u\n"RESET, chunk->perlinCave[blockPos.x][blockPos.z]);
	} else {
		ft_printf_fd(1, RED" -> Chunk not exist\n"RESET);
	}

	// Camera *cam = &c->cam;
	// f32 xDir = cam->viewVector[0];
	// // f32 yDir = cam->viewVector[1];
	// f32 zDir = cam->viewVector[2];

	// char *NordSouth = "Unknown";
	// if (xDir >= 0.0f) {
	// 	NordSouth = CYAN"Nord"RESET;
	// } else if (xDir < 0.0f) {
	// 	NordSouth = RED"South"RESET;
	// } 

	// char *EstWest = "Unknown";
	// if (zDir > 0.0f) {
	// 	EstWest = ORANGE"East"RESET;
	// } else if (zDir < 0.0f) {
	// 	EstWest = PINK"West"RESET;
	// }

	// ft_printf_fd(1, PINK"View vec: X|%f|, Z|%f| "RESET":\n[%s] [%s]\n", xDir, zDir, NordSouth, EstWest);

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
		{GLFW_KEY_L, act_change_cam_speed, SINGLE_PRESS},
		{GLFW_KEY_M, act_autoMoveTrigger, SINGLE_PRESS},
		{GLFW_KEY_N, act_autoRotateTrigger, SINGLE_PRESS},
	};
	u32 			max = (sizeof(key_actions) / sizeof(KeyAction));
	s32				state = GLFW_RELEASE;
 	Context 		*c = context;

	// mtx_lock(&c->gameMtx);

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

	if (c->autoMove) {
		move_camera_forward(&c->cam, c->cam.camSpeed);
	}
	if (c->autoRotate) {
		act_rotate_camera_left(c);
	}

	// mtx_unlock(&c->gameMtx);
}