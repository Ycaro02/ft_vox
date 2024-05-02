#include "../../include/vox.h"
#include "../../cube.h"

void drawCube(GLuint VAO, u32 vertex_nb, u32 cubeId) {
	glBindVertexArray(VAO);
	glDrawElementsInstanced(GL_TRIANGLES, vertex_nb, GL_UNSIGNED_INT, 0, cubeId);
	glBindVertexArray(0);
}

void drawAllCube(GLuint VAO, u32 nb_cube) {
	drawCube(VAO, 6*6, nb_cube);
	// for (u32 cubeId = 1; cubeId <= nb_cube; ++cubeId) {
	// 	drawCube(VAO, 6*6, cubeId);
	// }
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

    u32 v_size = sizeof(vertex) / sizeof(VertexTexture);
    cube->vertex = malloc(sizeof(VertexTexture) * v_size);
    ft_memcpy(cube->vertex, vertex, sizeof(VertexTexture) * v_size);
    cube->v_size = v_size;

    GLuint VAO, VBO, EBO;

    /* Generate vertex array object (VAO) */
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    /* Generate vertex buffer object (VBO) */
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);

    /* Generate element buffer object (EBO) */
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	/* Position attribute */
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTexture), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	/* Texture Coordinate attribute */
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTexture), (GLvoid*)sizeof(vec3));
	glEnableVertexAttribArray(2);


    vec3 *block_array = ft_calloc(sizeof(vec3), c->chunks->visible_block);
	u32 instanceCount = chunks_cube_get(c->chunks, block_array);


	GLuint instanceVBO;
	glGenBuffers(1, &instanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, instanceCount * sizeof(vec3), (GLfloat *)block_array[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glVertexAttribDivisor(1, 1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


    /* Unbind VBO and VAO */
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}