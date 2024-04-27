#ifndef HEADER_RENDER_H
#define HEADER_RENDER_H

#include "vox.h"

/* shader_utils.c */
void set_shader_var_vec4(GLuint shader_id, char *var_name, vec4_f32 vec);
void set_shader_var_mat4(GLuint shader_id, char *var_name, mat4_f32 data);
void set_shader_var_float(GLuint shader_id, char *var_name, float data);

#endif /* HEADER_RENDER_H */
