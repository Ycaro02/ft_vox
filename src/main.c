#include "../include/vox.h"			/* Main project header */

void renderScene(t_context *c, GLuint vao, GLuint shader_id) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
	glUseProgram(shader_id);
    drawCube(vao, c->chunks->nb_block);
    glFlush();
}

void fill_chunks(t_chunks *chunks)
{
    u32 count = 0;
    for (u32 i = 0; i < CHUNKS_WIDTH / 8; ++i) {
        for (u32 j = 0; j < CHUNKS_HEIGHT / 8; ++j) {
            for (u32 k = 0; k < CHUNKS_DEPTH / 8; ++k) {
                // ft_printf_fd(1, "B [%u][%u][%u]", i, j, k);
                /* chunks->block[i][0][k]*/
                t_block *block = &chunks->blocks[i][0][k];
                // ft_printf_fd(1, "D [%u][%u][%u] type: %u", block->x, block->y, block->z, block->type);
                block->x = i;
                block->y = j;
                block->z = k;
                block->type = STONE;
                ++count;
            }
        }
    }
    chunks->nb_block = count;
}

u32 chunks_cube_get(t_chunks *chunks, vec3_f32 *block_array)
{
    u32 idx = 0;

    for (u32 i = 0; i < CHUNKS_WIDTH; ++i) {
        for (u32 j = 0; j < CHUNKS_HEIGHT; ++j) {
            for (u32 k = 0; k < CHUNKS_HEIGHT; ++k) {
                /* chunks->block[i][0][k]*/
                t_block *block = &chunks->blocks[i][0][k];
                if (block->type != 0) {
                    block_array[idx][0] = i; 
                    block_array[idx][1] = j; 
                    block_array[idx][2] = k; 
                    ++idx;
                }
            }   
        }
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
    fill_chunks(context.chunks);

	GLuint vao = setupCubeVAO(&context, &context.cube);
	render.shader_id = load_shader(&render);
	context.shader_id = render.shader_id;
	context.cam = create_camera(45.0f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);

    mat4_identity(context.cube.rotation);

    while (!glfwWindowShouldClose(window)) {
		update_camera(&context, render.shader_id);
        renderScene(&context, vao, render.shader_id);

        glfwSwapBuffers(window);

        glfwPollEvents();
        handle_input(&context);
    }

    free(context.chunks);

    glfwTerminate();
    return 0;
}
