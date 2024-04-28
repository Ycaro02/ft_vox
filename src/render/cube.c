#include "../../include/vox.h"


void drawCube(GLuint vao)
{
    // Draw the cube using triangle fans
    glBindVertexArray(vao);
    for (int i = 0; i < 6; ++i) {
        glDrawArrays(GL_TRIANGLE_FAN, i*6, 6);
    }
    glBindVertexArray(0);
}

GLuint setupCubeVAO(t_dataCube *cube) 
{
	// Define vertex for the cube
	/*	- Each face is starting with central vertex 
		- Then the 4 vertices of the face
		- The last vertex is the first vertex of the face to close the triangle fan
		- The order of the vertices is clockwise
	*/
	static const GLfloat vertices[] = {
		// Front face
		0.0f, 0.0f, 0.5f,
		-0.5f, -0.5f, 0.5f,
		0.5f, -0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,
		-0.5f, 0.5f, 0.5f,
		-0.5f, -0.5f, 0.5f,
		// Back face
		0.0f, 0.0f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, 0.5f, -0.5f,
		-0.5f, 0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		// Top face
		0.0f, 0.5f, 0.0f,
		-0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, -0.5f,
		-0.5f, 0.5f, -0.5f,
		-0.5f, 0.5f, 0.5f,
		// Bottom face
		0.0f, -0.5f, 0.0f,
		-0.5f, -0.5f, 0.5f,
		0.5f, -0.5f, 0.5f,
		0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, 0.5f,
		// Left face
		-0.5f, 0.0f, 0.0f,
		-0.5f, -0.5f, 0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, 0.5f, -0.5f,
		-0.5f, 0.5f, 0.5f,
		-0.5f, -0.5f, 0.5f,
		// Right face
		0.5f, 0.0f, 0.0f,
		0.5f, -0.5f, 0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, 0.5f, -0.5f,
		0.5f, 0.5f, 0.5f,
		0.5f, -0.5f, 0.5f
	};
	u32 v_size = sizeof(vertices) / sizeof(GLfloat);
	cube->vertex = malloc(sizeof(GLfloat) * v_size);
	ft_memcpy(cube->vertex, vertices, sizeof(GLfloat) * v_size);
	cube->v_size = v_size;


    GLuint VAO, VBO;

    /* Generate vertex array object (VAO) */
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    /* Generate vertex buffer object (VBO) */
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    /* Specify vertex attribute pointers */
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    /* Unbind VBO and VAO */
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return (VAO);
}