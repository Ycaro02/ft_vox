#ifndef HEADER_RENDER_H
#define HEADER_RENDER_H

#include "vox.h"

typedef struct s_render {
	t_camera		cam;			/* camera structure */
    GLuint			vao;			/* vertex array object */
	GLuint			vbo;			/* vertex buffer object */
	GLuint			ebo;			/* element buffer object */
	GLuint			shader_id;		/* shader program id */
} t_render;

/* shader_utils.c */
void set_shader_var_vec4(GLuint shader_id, char *var_name, vec4_f32 vec);
void set_shader_var_mat4(GLuint shader_id, char *var_name, mat4_f32 data);
void set_shader_var_float(GLuint shader_id, char *var_name, float data);
GLuint load_shader(t_render *c);


#endif /* HEADER_RENDER_H */
