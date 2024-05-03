#include "../../include/vox.h"

void set_shader_var_vec4(GLuint shader_id, char *var_name, vec4 vec) {
	GLint var_loc = glGetUniformLocation(shader_id, var_name);
	glUniform4f(var_loc, vec[0], vec[1], vec[2], vec[3]);

}

void set_shader_var_mat4(GLuint shader_id, char *var_name, mat4 data)
{
	GLint var_loc = glGetUniformLocation(shader_id, var_name);
	glUniformMatrix4fv(var_loc, 1, GL_FALSE, (GLfloat *)data);
}

void set_shader_var_float(GLuint shader_id, char *name, float value)
{
	GLint loc = glGetUniformLocation(shader_id, name);
	glUniform1f(loc, value);
}

char *load_shader_file(char *path)
{
	char **file = sstring_load_file(path);
	char *shader_src = ft_strdup("");


	if (!file) {
		ft_printf_fd(2, RED"Error: Failed to load shader file\n"RESET);
		return (NULL);
	}
	for (u32 i = 0; file[i]; ++i) {
		shader_src = ft_strjoin_free(shader_src, file[i], 'f');
		shader_src = ft_strjoin_free(shader_src, "\n", 'f');
	}

	free_double_char(file);

	// ft_printf_fd(1, "Shader src: %s\n", shader_src);
	return (shader_src);
}

GLuint load_shader(t_render *c)
{
	char *vertex_shader = load_shader_file(CUBE_VERTEX_SHADER);
	char *fragment_shader = load_shader_file(CUBE_FRAGMENT_SHADER);
	
	/* create shader */
	GLuint frag_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	GLuint frag_pixel_shader = glCreateShader(GL_FRAGMENT_SHADER);

	/* compile shader */
	glShaderSource(frag_vertex_shader, 1, (const char **)&vertex_shader, NULL);
	glCompileShader(frag_vertex_shader);

	glShaderSource(frag_pixel_shader, 1, (const char **)&fragment_shader, NULL);
	glCompileShader(frag_pixel_shader);

	c->shader_id = glCreateProgram();
	
	/* Attach and link shader program  */
	glAttachShader(c->shader_id , frag_vertex_shader);
	glAttachShader(c->shader_id , frag_pixel_shader);
	
	glLinkProgram(c->shader_id);

	GLint succes = 0;
	glGetProgramiv(c->shader_id , GL_LINK_STATUS, &succes);
	if (!succes) {
		GLchar data[1024];
		ft_bzero(data, 1024);
		glGetProgramInfoLog(c->shader_id , 512, NULL, data);
		ft_printf_fd(2, "Shader program log: %s\n", data);
	} else {
		ft_printf_fd(1, "Shader program linked\n");
	}

	glUseProgram(c->shader_id);

	/* delete shader tocheck */
	glDeleteShader(frag_vertex_shader);
	glDeleteShader(frag_pixel_shader);

	/* delete ressource */
	free(vertex_shader);
	free(fragment_shader);
	return (c->shader_id);
}