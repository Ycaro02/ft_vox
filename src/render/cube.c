#include "../../include/vox.h"

void drawFace(GLuint VAO, u32 count, u32 cubeId) {
	glBindVertexArray(VAO);
	glDrawElementsInstanced(GL_TRIANGLE_FAN, count, GL_UNSIGNED_INT, 0, cubeId);
	glBindVertexArray(0);
}

void drawCube(GLuint VAO) {
	for (u32 cubeId = 1; cubeId <= 5; ++cubeId) {
		for (u32 i = 0; i < 6; ++i) {
			drawFace(VAO, i*4, cubeId);
		}
	}
}

GLuint setupCubeVAO(t_dataCube *cube) {
	static const GLfloat vertices[] = {
		/* Front face */
		-0.5f, -0.5f, 0.5f,
		0.5f, -0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,
		-0.5f, 0.5f, 0.5f,

		/* Back face */
		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, 0.5f, -0.5f,
		-0.5f, 0.5f, -0.5f,

		/* Top face */
		-0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, -0.5f,
		-0.5f, 0.5f, -0.5f,

		/* Bottom face */
		-0.5f, -0.5f, 0.5f,
		0.5f, -0.5f, 0.5f,
		0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,

		/* Left face */
		-0.5f, -0.5f, 0.5f,
		-0.5f, 0.5f, 0.5f,
		-0.5f, 0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,

		/* Right face */
		0.5f, -0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, -0.5f,
		0.5f, -0.5f, -0.5f
	};

	static const GLuint indices[] = {
		0, 1, 2, 3, 0,   // Front face
		4, 5, 6, 7, 4,   // Back face
		8, 9, 10, 11, 8, // Top face
		12, 13, 14, 15, 12, // Bottom face
		16, 17, 18, 19, 16, // Left face
		20, 21, 22, 23, 20  // Right face
	};

	// static const GLfloat texCoords[] = {
	// 	/* Front face */
	// 	0.0f, 0.0f,
	// 	1.0f, 0.0f,
	// 	1.0f, 1.0f,
	// 	0.0f, 1.0f,

	// 	/* Back face */
	// 	0.0f, 0.0f,
	// 	1.0f, 0.0f,
	// 	1.0f, 1.0f,
	// 	0.0f, 1.0f,

	// 	/* Top face */
	// 	0.0f, 1.0f,
	// 	1.0f, 1.0f,
	// 	1.0f, 0.0f,
	// 	0.0f, 0.0f,

	// 	/* Bottom face */
	// 	0.0f, 0.0f,
	// 	1.0f, 0.0f,
	// 	1.0f, 1.0f,
	// 	0.0f, 1.0f,

	// 	/* Left face */
	// 	0.0f, 0.0f,
	// 	1.0f, 0.0f,
	// 	1.0f, 1.0f,
	// 	0.0f, 1.0f,

	// 	/* Right face */
	// 	0.0f, 0.0f,
	// 	1.0f, 0.0f,
	// 	1.0f, 1.0f,
	// 	0.0f, 1.0f
	// };


    u32 v_size = sizeof(vertices) / sizeof(GLfloat);
    cube->vertex = malloc(sizeof(GLfloat) * v_size);
    ft_memcpy(cube->vertex, vertices, sizeof(GLfloat) * v_size);
    cube->v_size = v_size / 3;

    GLuint VAO, VBO, EBO;

    /* Generate vertex array object (VAO) */
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    /* Generate vertex buffer object (VBO) */
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    /* Generate element buffer object (EBO) */
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    /* Specify vertex attribute pointers */
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);


	u32 instanceCount = 5;
	vec3_f32 instancePositions[5] = {
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 1.0f},
		{0.0f, 0.0f, 2.0f},
		{0.0f, 0.0f, 3.0f},
		{0.0f, 0.0f, 4.0f}
	};
	GLuint instanceVBO;
	glGenBuffers(1, &instanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, instanceCount * sizeof(vec3_f32), &instancePositions[0], GL_STATIC_DRAW);
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