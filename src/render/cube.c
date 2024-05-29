#include "../../include/vox.h"
#include "../../include/cube.h"

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

FaceCubeModel *cubeFaceVAOinit() {
	FaceCubeModel *cubeFace = NULL;
    static const VertexTexture backFace[] = { CUBE_BACK_FACE_VERTEX };
    static const VertexTexture frontFace[] = { CUBE_FRONT_FACE_VERTEX };
    static const VertexTexture leftFace[] = { CUBE_LEFT_FACE_VERTEX };
    static const VertexTexture rightFace[] = { CUBE_RIGHT_FACE_VERTEX };
    static const VertexTexture bottomFace[] = { CUBE_BOTTOM_FACE_VERTEX };
    static const VertexTexture topFace[] = { CUBE_TOP_FACE_VERTEX };

    static const vec3_u32 backIndices[] = { CUBE_BACK_FACE(0, 1, 2, 3) };
    static const vec3_u32 frontIndices[] = { CUBE_FRONT_FACE(0, 1, 2, 3) };
    static const vec3_u32 leftIndices[] = { CUBE_LEFT_FACE(0, 1, 2, 3) };
    static const vec3_u32 rightIndices[] = { CUBE_RIGHT_FACE(0, 1, 2, 3) };
    static const vec3_u32 bottomIndices[] = { CUBE_BOTTOM_FACE(0, 1, 2, 3) };
    static const vec3_u32 topIndices[] = { CUBE_TOP_FACE(0, 1, 2, 3) };

    const VertexTexture *faces[] = { backFace, frontFace, leftFace, rightFace, bottomFace, topFace };
    const vec3_u32 *indices[] = { backIndices, frontIndices, leftIndices, rightIndices, bottomIndices, topIndices };
    size_t vertexSizes[] = { sizeof(backFace), sizeof(frontFace), sizeof(leftFace), sizeof(rightFace), sizeof(bottomFace), sizeof(topFace) };
    size_t indexSizes[] = { sizeof(backIndices), sizeof(frontIndices), sizeof(leftIndices), sizeof(rightIndices), sizeof(bottomIndices), sizeof(topIndices) };

	if (!(cubeFace = malloc(sizeof(FaceCubeModel) * FACE_VERTEX_ARRAY_SIZE))) {
		return (NULL);
	}

    for (int i = 0; i < FACE_VERTEX_ARRAY_SIZE; ++i) {
    	/* Generate and bind vertex array object (VAO) */
		glGenVertexArrays(1, &cubeFace[i].VAO);
        glBindVertexArray(cubeFace[i].VAO);

    	/* Generate and bind vertex buffer object (VBO) */
        cubeFace[i].VBO = bufferGlCreate(GL_ARRAY_BUFFER, vertexSizes[i], (void*)faces[i]);
    	
		/* Generate and bind element buffer object (EBO) */
        cubeFace[i].EBO = bufferGlCreate(GL_ELEMENT_ARRAY_BUFFER, indexSizes[i], (void*)indices[i]);
		
		/* Vertex attribute */
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTexture), (GLvoid*)0);
        glEnableVertexAttribArray(0);

		/* Texture Coordinate attribute */
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTexture), (GLvoid*)sizeof(vec3));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
    }
	return (cubeFace);
}