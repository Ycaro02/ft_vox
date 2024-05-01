#include "../include/vox.h"			/* Main project header */

void renderScene(t_context *c, GLuint vao, GLuint shader_id) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
	glUseProgram(shader_id);
    drawCube(vao, c->chunks->nb_block);
    glFlush();
}

size_t fill_sub_chunks(t_sub_chunks *sub_chunk)
{
    for (u32 i = 0; i < SUB_CHUNKS_WIDTH / 2; ++i) {
        for (u32 j = 0; j < 1; ++j) {
            for (u32 k = 0; k < SUB_CHUNKS_DEPTH / 2; ++k) {
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

u32 chunks_cube_get(t_chunks *chunks, vec3 *block_array)
{
    s8 next = TRUE;
	u32 idx = 0;

	hashMap_it it = hashmap_iterator(chunks->sub_chunks[0].block_map);
	next = hashmap_next(&it);
	while (next) {
		t_block *block = (t_block *)it.value;
		block_array[idx][0] = block->x;
		block_array[idx][1] = block->y;
		block_array[idx][2] = block->z;
		++idx;
		next = hashmap_next(&it);
	}

    ft_printf_fd(1, "nb cube %u\n", chunks->nb_block);
    return (chunks->nb_block);
}


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
	context.chunks->sub_chunks[0].block_map = hashmap_init(50, hashmap_entry_free);
    
	context.chunks->nb_block = fill_sub_chunks(&context.chunks->sub_chunks[0]);

	GLuint vao = setupCubeVAO(&context, &context.cube);
	render.shader_id = load_shader(&render);
	context.shader_id = render.shader_id;
	context.cam = create_camera(45.0f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);


    glm_mat4_identity(context.cube.rotation);

    while (!glfwWindowShouldClose(window)) {
		update_camera(&context, render.shader_id);
        renderScene(&context, vao, render.shader_id);

        glfwSwapBuffers(window);

        glfwPollEvents();
        handle_input(&context);
    }

	hashmap_destroy(context.chunks->sub_chunks[0].block_map);

    free(context.chunks);
    glfwTerminate();
    return 0;
}
