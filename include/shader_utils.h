#ifndef HEADER_SHADER_UTILS_H
#define HEADER_SHADER_UTILS_H

#include "typedef_struct.h"

/* shader_utils.c */
void 	set_shader_var_vec4(GLuint shader_id, char *var_name, vec4 vec);
void 	set_shader_var_mat4(GLuint shader_id, char *var_name, mat4 data);
void 	set_shader_var_float(GLuint shader_id, char *var_name, float data);
void	set_shader_var_vec3(GLuint shader_id, char *name, vec3 vec);
GLuint	load_shader(char *vertexShader, char *fragmentShader);

#endif /* HEADER_SHADER_UTILS_H */
