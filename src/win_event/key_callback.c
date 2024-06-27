#include "../../include/camera.h"
#include "../../include/vox.h"
#include "../../include/world.h"
#include "../../include/chunks.h"
#include "../../rsc/perlin_noise/include/perlin_noise.h"
#include "../../include/render_chunks.h"
#include "../../include/block.h"
#include "../../include/shader_utils.h"
#include "../../include/win_event.h"

/* Escapte Key : ESC */
void act_escape(Context *c) {
    glfwSetWindowShouldClose(c->win_ptr, GL_TRUE);
}

void act_change_cam_speed(Context *c) {
	c->cam->camSpeed = (c->cam->camSpeed == CAM_BASE_SPEED) ? (CAM_HIGHT_SPEED) : CAM_BASE_SPEED;
}

void act_autoMoveTrigger(Context *c) {
	c->autoMove = (c->autoMove == FALSE) ? TRUE : FALSE;
}

void act_autoRotateTrigger(Context *c) {
	c->autoRotate = (c->autoRotate == FALSE) ? TRUE : FALSE;
}

void act_displayDataTrigger(Context *c) {
	c->displayData.displayBool = (c->displayData.displayBool == FALSE) ? TRUE : FALSE;
}

void act_displayUnderGroundTrigger(Context *c) {
	c->displayUndergroundBlock = (c->displayUndergroundBlock == FALSE) ? TRUE : FALSE;
}

/* Zoom : W */
void act_zoom(Context *c) {
    move_camera_forward(c->cam, c->cam->camSpeed);
}

/* Unzoom : S */
void act_unzoom(Context *c) {
    move_camera_backward(c->cam, c->cam->camSpeed);
}

/* Unzoom: D */
void act_move_right(Context *c) {
	straf_camera(c->cam, c->cam->camSpeed, DIR_RIGHT);
}

/* Unzoom: A */
void act_move_left(Context *c) {
	straf_camera(c->cam, c->cam->camSpeed, DIR_LEFT);
}

/* Rotate camera left : LEFT */
void act_rotate_camera_left(Context *c) {
	rotate_camera(c->cam, 1.0f, VEC3_ROTATEY);
	// rotate_camera(c->cam, 3.0f, VEC3_ROTATEY);
}

/* Rotate camera right: RIGHT */
void act_rotate_camera_right(Context *c) {
    rotate_camera(c->cam, -1.0f, VEC3_ROTATEY);
    // rotate_camera(c->cam, -3.0f, VEC3_ROTATEY);
}

/* Rotate camera top UP */
void act_rotate_camera_top(Context *c) {
	rotateTopBot(c->cam, -1.0f);
}

/* Rotate camera down : DOWN */
void act_rotate_camera_down(Context *c) {
	rotateTopBot(c->cam, 1.0f);
}

/* Up camera : SPACE */
void act_up_camera(Context *c) {
    move_camera_up(c->cam, c->cam->camSpeed);
    // move_camera_up(c->cam, CAM_UP_DOWN);
}

/* Down camera : Q */
void act_down_camera(Context *c) {
    move_camera_up(c->cam, -c->cam->camSpeed);
    // move_camera_up(c->cam, -CAM_UP_DOWN);
}

/* Reset cam : ENTER */
void act_reseCamera(Context *c) {
    reseCamera(c);
}


void displayPerlinNoise(PerlinData perlinData) {
	// ft_printf_fd(1, PINK"Perlin Idx0 : X:[%d] Z:[%d]\n"RESET, perlinData.x0, perlinData.z0);
	// ft_printf_fd(1, PINK"Perlin Idx1 : X:[%d] Z:[%d]\n"RESET, perlinData.x1, perlinData.z1);
	ft_printf_fd(1, CYAN"Perlin data:\n-----------------\n"RESET);
	ft_printf_fd(1, RED"Given X:|%d|, Given Z:|%d|\n"RESET, perlinData.givenX, perlinData.givenZ);
	ft_printf_fd(1, ORANGE"Continental Val:|%f|\n", perlinData.valContinent);
	ft_printf_fd(1, "Erosion Val:|%f|\n", perlinData.valErosion);
	ft_printf_fd(1, "PeaksValley Val:|%f|\n", perlinData.valPeaksValley);
	ft_printf_fd(1, RESET""PINK"Normalise:|%d| -> ", perlinData.normalise);
	ft_printf_fd(1, "Combined:|%f|\n"RESET, perlinData.valCombined);
	ft_printf_fd(1, GREEN"Humidity:|%f|\n", perlinData.valHumidity);
	ft_printf_fd(1, "Temperature:|%f|\n"RESET, perlinData.valTemperature);
	ft_printf_fd(1, CYAN"Perlin End :\n-----------------\n"RESET);
}


void displayChunkData(Context *c, Chunks *chunk){
	RenderChunks *renderChunk = chunk->render;
	ft_printf_fd(1, "Chunk data:\n-----------------\n"RESET);
	if (renderChunk) {
		char *isRender = (chunksIsRenderer(c->world->renderChunksMap, renderChunk->chunkID)) ? GREEN"YES"RESET : RED"NO"RESET;
		ft_printf_fd(1, YELLOW"Is in render map == "RESET"%s\n", isRender);
		for (u32 i = 0; i < 6; ++i) {
			// ft_printf_fd(1, YELLOW"Face:|%u| Count %u-> "RESET, i, renderChunk->faceCount[i]);
			// ft_printf_fd(1, YELLOW"Trsp:|%u| Count %u-> "RESET, i, renderChunk->trspFaceCount[i]);
			ft_printf_fd(1, ORANGE"Face VBO %u, TypeVBO %u\n"RESET, renderChunk->faceVBO[i], renderChunk->faceTypeVBO[i]);
			ft_printf_fd(1, ORANGE"Trsp VBO %u, TypeVBO %u\n"RESET, renderChunk->trspFaceVBO[i], renderChunk->trspTypeVBO[i]);
		}
		ft_printf_fd(1, YELLOW"Top Face Water:|%u|\n"RESET, renderChunk->topFaceWaterCount);
		ft_printf_fd(1, ORANGE"Top Face Water VBO:|%u|, Top Face Water Type VBO:|%u|\n"RESET, renderChunk->topFaceWaterVBO, renderChunk->topFaceWaterTypeVBO);
		ft_printf_fd(1, GREEN"Last update %u\n"RESET, (suseconds_t)(renderChunk->lastUpdate - get_ms_time()));
	} else {
		ft_printf_fd(1, RED"Render chunk is NULL\n"RESET);
	}
	ft_printf_fd(1, CYAN"Chunk End :\n-----------------\n"RESET);

}


// void context_DisplayData_debug(Context *c) {
// 	ft_printf_fd(1, "Display data:\n-----------------\n"RESET);
// 	ft_printf_fd(1, RED"Local Block Pos: X|%d|, Z|%d|\n"RESET, c->displayData.blockPos.x, c->displayData.blockPos.z);
// 	ft_printf_fd(1, "Noise data:\n-----------------\n"RESET);
// 	ft_printf_fd(1, ORANGE"Continental Val:|%f|\n", c->displayData.noiseData.valContinental);
// 	ft_printf_fd(1, "Erosion Val:|%f|\n", c->displayData.noiseData.valErosion);
// 	ft_printf_fd(1, "PeaksValley Val:|%f|\n", c->displayData.noiseData.valPeaksValley);
// 	ft_printf_fd(1, "Combined Val:|%f|\n", c->displayData.noiseData.valCombined);
// 	ft_printf_fd(1, GREEN"Humidity Val:|%f|\n", c->displayData.noiseData.valHumidity);
// 	ft_printf_fd(1, "Temperature Val:|%f|\n"RESET, c->displayData.noiseData.valTemperature);
// 	ft_printf_fd(1, CYAN"Display data end :\n-----------------\n"RESET);
// }


void displayBlockPosition(Chunks *chunk, BlockPos blockPos) {
	ft_printf_fd(1, PURPLE"-----------------\n"RESET);
	ft_printf_fd(1, YELLOW"Chunk : X|%d|, Z|%d|\n"RESET, chunk->x, chunk->z);
	ft_printf_fd(1, ORANGE"World : X|%d|, Z|%d|\n"RESET, blockPos.x + 16 * chunk->x, blockPos.z + 16 * chunk->z);
	ft_printf_fd(1,   CYAN"Local : X|%d|, Z|%d|\n"RESET, blockPos.x, blockPos.z);
	ft_printf_fd(1, PURPLE"-----------------\n"RESET);

}

void testChunksExist(Context *c) {
	BlockPos chunkPos = CHUNKS_MAP_ID_GET(c->cam->chunkPos[0], c->cam->chunkPos[2]);
	// display_camera_value(c);
	ft_printf_fd(1, YELLOW"\nTest for chunk:"RESET" "ORANGE"X|%d| Z|%d|"RESET, chunkPos.y, chunkPos.z);
	Chunks *chunks = hashmap_get(c->world->chunksMap, chunkPos);
	if (chunks) {
		ft_printf_fd(1, GREEN" -> Chunk exist\n"RESET);
		BlockPos blockPos = {0};
		blockLocalPosFromCam(c->cam->position, &blockPos);
		Chunks	*chunk = hashmap_get(c->world->chunksMap, chunkPos);
		ft_printf_fd(1, CYAN"Cam position: X|%f, Y:%f Z:|%f\n"RESET, c->cam->position[0], c->cam->position[1], c->cam->position[2]);
		displayBlockPosition(chunk, blockPos);
		displayPerlinNoise(chunk->noiseData[blockPos.x][blockPos.z]);
		// context_DisplayData_debug(c);
		// if (undergroundBoolUpdate(c, chunk)) {
		// 	ft_printf_fd(1, GREEN" -> Camera is not underground\n"RESET);
		// 	c->world->undergroundBlock->isUnderground = FALSE;
		// } else {
		// 	ft_printf_fd(1, RED" -> Camera is underground\n"RESET);
		// 	c->world->undergroundBlock->isUnderground = TRUE;
		// }
		mtx_lock(&c->vboToCreateMtx);
		displayChunkData(c, chunk);
		mtx_unlock(&c->vboToCreateMtx);
		/* TO restore now u8 val */
		// ft_printf_fd(1, RED"Perlin cave\n");
		// ft_printf_fd(1, "Val: %u\n"RESET, chunk->perlinCave[blockPos.x][blockPos.z]);
	} else {
		ft_printf_fd(1, RED" -> Chunk not exist\n"RESET);
	}

	// ft_printf_fd(1, PINK"View vec: X|%f|, Z|%f| "RESET":\n[%s] [%s]\n", xDir, zDir, NordSouth, EstWest);

}

/* Display cam data : C */
void act_display_camera_value(Context *c) {
    ft_printf_fd(1, CYAN"\nPos: %f, %f, %f\n"RESET, c->cam->position[0], c->cam->position[1],c->cam->position[2]);
	ft_printf_fd(1, PINK"Chunk X[%d], Z[%d], Y:|%d|\n", c->cam->chunkPos[0], c->cam->chunkPos[2], c->cam->chunkPos[1]);
	// display_camera_value(c->cam);
}

/* Change polygon mode : P */
void act_change_polygon_mode(Context *c) {
	/* To store in context structure */
    static u8 fill_mode = 1;
	
	(void)c;
    fill_mode = !fill_mode;
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE + fill_mode);
}

/*
 * Key Bindings and Actions
 * -------------------------------------------------------------------------------------------------|
 * |   Key    |         Action           		|                Description                		|
 * -------------------------------------------------------------------------------------------------|
 * |  ESC     | act_escape               		|		Closes the application window.				|
 * |  P       | act_change_polygon_mode  		|		Toggles polygon rendering mode.				|
 * |  W       | act_zoom                 		|		Moves the camera forward.					|
 * |  S       | act_unzoom               		|		Moves the camera backward.					|
 * |  D       | act_move_right           		|		Moves the camera to the right.				|
 * |  A       | act_move_left            		|		Moves the camera to the left.				|
 * |  SPACE   | act_up_camera            		|		Moves the camera upward.					|
 * |  Q       | act_down_camera          		|		Moves the camera downward.					|
 * |  LEFT    | act_rotate_camera_left   		|		Rotates the camera to the left.				|
 * |  RIGHT   | act_rotate_camera_right  		|		Rotates the camera to the right.			|
 * |  UP      | act_rotate_camera_top    		|		Rotates the camera upward.					|
 * |  DOWN    | act_rotate_camera_down   		|		Rotates the camera downward.				|
 * |  ENTER   | act_reseCamera           		|		Resets the camera position.					|
 * |  C       | act_display_camera_value 		|		Displays current camera position.			|
 * |  E       | testChunksExist          		|		Tests and displays chunk information.		|
 * |  L       | act_change_cam_speed     		|		Toggles camera movement speed.				|
 * |  M       | act_autoMoveTrigger      		|		Toggles automatic camera movement.			|
 * |  N       | act_autoRotateTrigger    		|		Toggles automatic camera rotation.			|
 * |  U       | act_displayUnderGroundTrigger	|		Toggles underground block display.			|
 * |  TAB     | act_displayDataTrigger   		|		Toggles debug data display.					|
 * |------------------------------------------------------------------------------------------------|
 */

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
		{GLFW_KEY_U, act_displayUnderGroundTrigger, SINGLE_PRESS},
		{GLFW_KEY_TAB, act_displayDataTrigger, SINGLE_PRESS}
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
		move_camera_forward(c->cam, c->cam->camSpeed);
	}
	if (c->autoRotate) {
		act_rotate_camera_left(c);
	}

	// mtx_unlock(&c->gameMtx);
}