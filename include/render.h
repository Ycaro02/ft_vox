#ifndef HEADER_RENDER_H
#define HEADER_RENDER_H

#include "vox.h"

// typedef struct s_render {
//     GLuint			vao;			/* vertex array object */
// 	GLuint			vbo;			/* vertex buffer object */
// 	GLuint			ebo;			/* element buffer object */
// 	// GLuint			cubeShaderID;		/* shader program id */
// } t_render;

/* Model structure */
typedef struct s_modelCube {
	vec3			*vertex;		/* vertex array, give to openGL context */
	u32				v_size;			/* vertex size */
	mat4			rotation;		/* rotation matrix */
}	ModelCube;

/* shader_utils.c */
void 	set_shader_var_vec4(GLuint shader_id, char *var_name, vec4 vec);
void 	set_shader_var_mat4(GLuint shader_id, char *var_name, mat4 data);
void 	set_shader_var_float(GLuint shader_id, char *var_name, float data);
GLuint	load_shader(char *vertexShader, char *fragmentShader);


/* obj_rotate.c */
void rotate_object_around_center(ModelCube* cube, vec3 rotate_vec, float angle, GLuint shader_id); 
void get_obj_center(ModelCube* cube, vec3 center);

#endif /* HEADER_RENDER_H */
