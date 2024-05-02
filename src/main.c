#include "../include/vox.h"			/* Main project header */

void renderScene(t_context *c, GLuint vao, GLuint shader_id) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
	glUseProgram(shader_id);
    drawAllCube(vao, c->chunks->visible_block);
    glFlush();
}

size_t BRUT_fill_subchunks(t_sub_chunks *sub_chunk)
{
    for (u32 i = 0; i < SUB_CHUNKS_WIDTH; ++i) {
        for (u32 j = 0; j < SUB_CHUNKS_HEIGHT; ++j) {
            for (u32 k = 0; k < SUB_CHUNKS_DEPTH; ++k) {
                t_block *block = ft_calloc(sizeof(t_block), 1);
				if (!block) {
					ft_printf_fd(2, "Failed to allocate block\n");
					return (0);
				}
                block->x = i;
                block->y = j;
                block->z = k;
                block->type = STONE;
				hashmap_set_entry(sub_chunk->block_map, (t_block_pos){i, j, k}, block);
            }
        }
    }
	return (hashmap_size(sub_chunk->block_map));
}

#define SUBCHUNKS_DISPLAY 2U

void BRUT_FillChunks(t_chunks *chunks) {
	for (u32 i = 0; i < SUBCHUNKS_DISPLAY; ++i) {
		chunks->nb_block += BRUT_fill_subchunks(&chunks->sub_chunks[i]);
		chunks->visible_block += checkHiddenBlock(chunks, i);
	}
}

u32 chunks_cube_get(t_chunks *chunks, vec3 *block_array)
{
    s8 next = TRUE;
	u32 idx = 0;


	for (u32 subID = 0; subID < SUBCHUNKS_DISPLAY; ++subID) {
		hashMap_it it = hashmap_iterator(chunks->sub_chunks[subID].block_map);
		next = hashmap_next(&it);
		while (next) {
			t_block *block = (t_block *)it.value;
			
			if (block->flag != BLOCK_HIDDEN) {
				block_array[idx][0] = block->x;
				block_array[idx][1] = block->y + (subID * SUB_CHUNKS_HEIGHT);
				block_array[idx][2] = block->z;
				++idx;
			}
			next = hashmap_next(&it);
		}
	}

    ft_printf_fd(1, GREEN"Renderer Cube %u\n"RESET, idx);
    return (idx);
}

void vox_destroy(t_context *c, GLuint *atlas)
{
    hashmap_destroy(c->chunks->sub_chunks[0].block_map);
    free(atlas);
    free(c->chunks);
    glfwTerminate();

}

enum AtlasId {
	ATLAS_DIRT_PINK=0, /* Pink to remove */
	ATLAS_SAND=1,
	ATLAS_STONE_CUT=2,
	ATLAS_BRICK=3,
	ATLAS_WOOD=4,
	ATLAS_STONE=5,
	ATLAS_DIRT=6,
	ATLAS_WOOD_PLANK=7, /* same here */
	ATLAS_DIRT2=8,
	ATLAS_GLASS=9,
	ATLAS_COBBLESTONE=10,
	ATLAS_FULL_GREY=11,
	ATLAS_STONE_CLEAN=12,
};

FT_INLINE void display_fps() {
	static double lastTime = 0.0f; 
	static int nbFrames = 0;

	if (lastTime == 0.0f) {
		lastTime = glfwGetTime();
	}
	double currentTime = glfwGetTime();
	nbFrames++;
	if (currentTime - lastTime >= 1.0) { // Si plus d'une seconde s'est écoulée
		// ft_printf_fd(1, ORANGE"%f ms/frame, %d FPS\n"RESET, (1000.0 / (double)nbFrames), nbFrames);
		printf("\r\033[K"ORANGE"%f ms/frame, %d FPS"RESET, (1000.0 / (double)nbFrames), nbFrames);
		fflush(stdout);
		nbFrames = 0;
		lastTime += 1.0;
	}
}

FT_INLINE void main_loop(t_context *context, GLuint vao, t_render *render) {

    while (!glfwWindowShouldClose(context->win_ptr)) {
		update_camera(context, render->shader_id);
        renderScene(context, vao, render->shader_id);

        glfwSwapBuffers(context->win_ptr);

        glfwPollEvents();
        handle_input(context);
		display_fps();
    }

}

#define HASHMAP_SIZE_100 151U
#define HASHMAP_SIZE_1000 1009U

int main() {
    t_context context;
    GLFWwindow* window;
	t_render render;

	ft_bzero(&render, sizeof(t_render));

    window = init_openGL_context();
    context.win_ptr = window;

    context.chunks = ft_calloc(sizeof(t_chunks), 1);
    if (!context.chunks) {
        return (1);
    }
    ft_printf_fd(1, "%u byte allocated\n", sizeof(t_chunks));
	context.chunks->sub_chunks[0].block_map = hashmap_init(HASHMAP_SIZE_1000, hashmap_entry_free);
	context.chunks->sub_chunks[1].block_map = hashmap_init(HASHMAP_SIZE_1000, hashmap_entry_free);
    
	BRUT_FillChunks(context.chunks);

	// context.chunks->nb_block = BRUT_fill_subchunks(&context.chunks->sub_chunks[0]);
	// context.chunks->visible_block = checkHiddenBlock(context.chunks, 0);

	GLuint vao = setupCubeVAO(&context, &context.cube);
	render.shader_id = load_shader(&render);
	context.shader_id = render.shader_id;
	context.cam = create_camera(45.0f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
    glm_mat4_identity(context.cube.rotation);

    GLuint *texture_atlas = load_texture_atlas();
	set_shader_texture(&context, texture_atlas, ATLAS_STONE);

	/* Disable VSync to avoid fps locking */
	// glfwSwapInterval(0);

	main_loop(&context, vao, &render);

    vox_destroy(&context, texture_atlas);
    return 0;
}
