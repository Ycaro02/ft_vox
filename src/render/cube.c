#include "../../include/vox.h"
#include "../../include/cube.h"
#include "../../include/render_chunks.h"
/**
 * @brief Draw a cube
 * @param VAO Vertex Array Object
 * @param vertex_nb number of vertex
 * @param cubeId cube id
*/
void drawCube(GLuint VAO, RenderChunks *render, u32 vertex_nb, u32 cubeId) {
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, render->instanceVBO);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glVertexAttribDivisor(1, 1);
	
    glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, render->typeBlockVBO);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
	glVertexAttribDivisor(3, 1);



	glDrawElementsInstanced(GL_TRIANGLES, vertex_nb, GL_UNSIGNED_INT, 0, cubeId);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

/**
 * @brief Draw all cube
 * @param VAO Vertex Array Object
 * @param nb_cube number of cube to draw 
*/	
void drawAllCube(GLuint VAO, RenderChunks *render) {
	u32 nb_cube = render->visibleBlock;

	drawCube(VAO, render, 6*6, nb_cube);
}

/**
 * @brief Get the offset of the block array
 * @param visible_block_array array of visible block
 * @param chunk_id chunk id
 * @return offset of the block array
*/
u32 blockArrayOffsetGet(u32 *visible_block_array, u32 chunk_id) {
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

GLuint setupCubeVAO(Context *c, ModelCube *cube) {
	(void)c;

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
	// glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTexture), (GLvoid*)sizeof(vec3));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTexture), (GLvoid*)sizeof(vec3));
	glEnableVertexAttribArray(2);
    glBindVertexArray(0);
    return (VAO);

	// vec3 *block_array = chunksToBlockArray(c, c->world->chunksMap);
	// if (!block_array) {
	// 	ft_printf_fd(1, RED"Error: block_array is NULL\n"RESET);
	// 	return (0);
	// }
	// /* Instance position */
	// GLuint instanceVBO = bufferGlCreate(GL_ARRAY_BUFFER, c->renderBlock * sizeof(vec3), (void *)block_array[0]);
	// free(block_array);
	// glBindBuffer(GL_ARRAY_BUFFER, 0);
	// glBindVertexArray(VAO);
	// glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	// glEnableVertexAttribArray(1);
	// glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	// glVertexAttribDivisor(1, 1);
    /* Unbind VBO and VAO */
    // glBindBuffer(GL_ARRAY_BUFFER, 0);
    // glBindVertexArray(0);
    // return (VAO);
}

// u32 *totalVisibleBlockGet(HashMap *chunksMap, u32 *visibleBlock)
// {
// 	u32 *visible_block_array = ft_calloc(sizeof(u32), hashmap_size(chunksMap));

// 	HashMap_it it = hashmap_iterator(chunksMap);
// 	s8 next = hashmap_next(&it); 
// 	u32 i = 0;

// 	while (next) {
// 		Chunks *chunks = (Chunks *)it.value;
// 		visible_block_array[i] = chunks->visible_block;
// 		*visibleBlock += chunks->visible_block; 
// 		++i;
// 		next = hashmap_next(&it);
// 	}
// 	return (visible_block_array);
// }

// /**
//  * @brief Get the block array
//  * @param c context
//  * @return block array
// */
// vec3 *getBlockArray(Context *c, HashMap *chunksMap, u32 *visibleBlockArr, u32 visibleBlock) {
// 	u32 instanceCount = 0;
//     vec3 *block_array = ft_calloc(sizeof(vec3), visibleBlock);
// 	HashMap_it it = hashmap_iterator(chunksMap);
// 	s8 next = hashmap_next(&it); 
// 	u32 i = 0;

// 	while (next) {
// 		Chunks *chunks = (Chunks *)it.value;
// 		u32 offset = blockArrayOffsetGet(visibleBlockArr, i); 
// 		instanceCount += chunksCubeGet(chunks, &block_array[offset], i);
// 		++i;
// 		next = hashmap_next(&it);
// 	}

// 	free(visibleBlockArr);
// 	ft_printf_fd(1, CYAN"instanceCount: %d\n"RESET, instanceCount);
// 	c->renderBlock = instanceCount;
// 	return (block_array);
// }

// vec3 *chunksToBlockArray(Context *c, HashMap *chunksMap) {
// 	u32 visibleBlock = 0;
// 	u32 *visibleBlockArr = totalVisibleBlockGet(chunksMap, &visibleBlock);
// 	if (!visibleBlockArr || visibleBlock == 0) {
// 		ft_printf_fd(1, "Error get visible block\n");
// 		return (NULL);
// 	}
// 	ft_printf_fd(1, GREEN"visibleBlock: %u\n"RESET, visibleBlock);
// 	return (getBlockArray(c, c->world->chunksMap, visibleBlockArr, visibleBlock));
// }
