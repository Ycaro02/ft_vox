#ifndef HEADER_CUBE_H
#define HEADER_CUBE_H

#include "cube_definition.h"

typedef struct PACKED_STRUCT s_vertex_texture {
    vec3_f32 position;
    vec3_f32 texCoord;
} VertexTexture;

/* Model structure */
typedef struct s_modelCube {
	vec3			*vertex;		/* vertex array, give to openGL context */
	u32				v_size;			/* vertex size */
	mat4			rotation;		/* rotation matrix */
}	ModelCube;

#define FACE_VERTEX_ARRAY_SIZE 6

typedef struct s_faceCubeModel {
	VertexTexture	*vertex;
	vec3_u32		*indices;
	GLuint			VAO;
	GLuint			VBO;
	GLuint			EBO;
} FaceCubeModel;

/* render/cube.c */
GLuint	setupCubeVAO(ModelCube *cube);

FaceCubeModel *cubeFaceVAOinit(void);

#endif /* HEADER_CUBE_H */
