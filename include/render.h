#ifndef HEADER_RENDER_H
#define HEADER_RENDER_H

#include "vox.h"

typedef struct s_render {
    GLuint			vao;			/* vertex array object */
	GLuint			vbo;			/* vertex buffer object */
	GLuint			ebo;			/* element buffer object */
	GLuint			shader_id;		/* shader program id */
} t_render;

/* Model structure */
typedef struct s_dataCube {
	vec3_f32		*vertex;		/* vertex array, give to openGL context */
	u32				v_size;			/* vertex size */
	// vec3_u32		*tri_face;		/* face array, give to openGL context, each vector is a triangle, each point represent index of vertex to link */
	// u32				tri_size;		/* face size */
	// vec2_f32		*texture_coord;	/* texture coordinates associated with a vertex */
	mat4_f32		rotation;		/* rotation matrix */
}	t_dataCube;


/* shader_utils.c */
void set_shader_var_vec4(GLuint shader_id, char *var_name, vec4_f32 vec);
void set_shader_var_mat4(GLuint shader_id, char *var_name, mat4_f32 data);
void set_shader_var_float(GLuint shader_id, char *var_name, float data);
GLuint load_shader(t_render *c);

/* cube.c */

/**
 * @brief Draw a cube
 * @param created vao vertex array object
*/
GLuint setupCubeVAO(t_dataCube *cube);

/**
 * @brief Setup the vertex array object for the cube
 * @param vao vertex array object
 * @return vertex array object
*/
void	drawCube(GLuint vao);

/* obj_rotate.c */
void rotate_object_around_center(t_dataCube* cube, vec3_f32 rotate_vec, float angle, GLuint shader_id); 
void get_obj_center(t_dataCube* cube, vec3_f32 center);

#endif /* HEADER_RENDER_H */
