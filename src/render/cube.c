#include "../../include/vox.h"
#include "../../include/cube.h"

void drawCube(GLuint VAO, u32 vertex_nb, u32 cubeId) {
	glBindVertexArray(VAO);
	glDrawElementsInstanced(GL_TRIANGLES, vertex_nb, GL_UNSIGNED_INT, 0, cubeId);
	glBindVertexArray(0);
}

void drawAllCube(GLuint VAO, u32 nb_cube) {
	drawCube(VAO, 6*6, nb_cube);
}

u32 get_block_arr_offset(u32 *visible_block_array, u32 chunk_id) {
	u32 offset = 0;
	for (u32 i = 0; i < chunk_id; ++i) {
		offset += visible_block_array[i];
	}
	return (offset);
}

/**
 * @brief Create VBO
 * @param size size of the buffer
 * @param data data to fill the buffer
 * @return vbo uint value
*/
GLuint bufferGlCreate(GLenum type, u32 size, void *data)
{
	GLuint	vbo;

	/* create and fill vbo */
	glGenBuffers(1, &vbo);
	glBindBuffer(type, vbo);
	glBufferData(type, size, data, GL_STATIC_DRAW);
	return (vbo);
}



vec3 *getBlockArray(t_context *c) {
	u32 visibleBlock = 0, instanceCount = 0;
	u32 visible_block_array[TEST_CHUNK_MAX] = {0};

	for (u32 i = 0; i < TEST_CHUNK_MAX; ++i) {
		visible_block_array[i] = c->chunks[i].visible_block;
		visibleBlock += c->chunks[i].visible_block;
	}

    vec3 *block_array = ft_calloc(sizeof(vec3), visibleBlock);

	for (u32 i = 0; i < TEST_CHUNK_MAX; ++i) {
		u32 offset = get_block_arr_offset(visible_block_array, i); 
		instanceCount += chunks_cube_get(&c->chunks[i], &block_array[offset], i);
	}
	ft_printf_fd(1, CYAN"instanceCount: %d\n"RESET, instanceCount);
	c->renderBlock = instanceCount;
	return (block_array);
}

GLuint setupCubeVAO(t_context *c, t_modelCube *cube) {
	static const VertexTexture vertex[] = {
		CUBE_BACK_FACE_VERTEX,
		CUBE_FRONT_FACE_VERTEX,
		CUBE_LEFT_FACE_VERTEX,
		CUBE_RIGHT_FACE_VERTEX,
		CUBE_BOTTOM_FACE_VERTEX,
		CUBE_TOP_FACE_VERTEX,
	};

	static const vec3_u32 indices[] = {
		CUBE_BACK_FACE(0, 1, 2, 3),
		CUBE_FRONT_FACE(4, 5, 6, 7),
		CUBE_LEFT_FACE(8, 9, 10, 11),
		CUBE_RIGHT_FACE(12, 13, 14, 15),
		CUBE_BOTTOM_FACE(16, 17, 18, 19),
		CUBE_TOP_FACE(20, 21, 22, 23),
	};

    static u32 v_size = sizeof(vertex) / sizeof(VertexTexture);
    
	if (!cube->vertex) {
		cube->vertex = malloc(sizeof(VertexTexture) * v_size);
		ft_memcpy(cube->vertex, vertex, sizeof(VertexTexture) * v_size);
		cube->v_size = v_size;
	}

    GLuint VAO;

    /* Generate vertex array object (VAO) */
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    /* Generate vertex buffer object (VBO) */
	bufferGlCreate(GL_ARRAY_BUFFER, sizeof(vertex), (void *)vertex);

    /* Generate element buffer object (EBO) */
	bufferGlCreate(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), (void *)indices);

	/* Position attribute */
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTexture), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	/* Texture Coordinate attribute */
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTexture), (GLvoid*)sizeof(vec3));
	glEnableVertexAttribArray(2);


	vec3 *block_array = getBlockArray(c);
	if (!block_array) {
		ft_printf_fd(1, RED"Error: block_array is NULL\n"RESET);
		return (0);
	}
	
	/* Instance position */
	GLuint instanceVBO = bufferGlCreate(GL_ARRAY_BUFFER, c->renderBlock * sizeof(vec3), (void *)block_array[0]);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glVertexAttribDivisor(1, 1);

    /* Unbind VBO and VAO */
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return (VAO);
}