#include "../../include/vox.h"

GLuint init_openGL_texture(u8 *data, u32 width, u32 height, u16 texture_type)
{
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	/* Set the texture wrapping parameters */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	/* Set texture filtering parameters */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);	// Set texture filtering to GL_LINEAR
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	/* Load the texture */
	glTexImage2D(GL_TEXTURE_2D, 0, texture_type, width, height, 0, texture_type, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	// GLuint texture_loc = glGetUniformLocation(model->shader_id, "texture1");
	// glUniform1i(texture_loc, 0);

	return (texture);
}


GLuint *load_gl_texture_atlas(t_list *atlas, int type)
{
    u32 lst_size = ft_lstsize(atlas);
    u16 data_type = GL_RGB;
    if (type == 4) {
        data_type = GL_RGBA;
    }
    GLuint *texture_id = malloc(sizeof(GLuint) * lst_size);
    u32 i = 0;

    for (t_list *curr = atlas; curr; curr = curr->next) {
        u8 *data = curr->content;
        texture_id[i] = init_openGL_texture(data, 16, 16, data_type);
        ++i;
    }
    return (texture_id);
}

GLuint *load_texture_atlas() {
    int w,h,type;

    u8 *texture = parse_bmp_file(TEXTURE_ATLAS_PATH, &w, &h, &type);
    if (!texture) {
        ft_printf_fd(2, "Failed to load texture\n");
        return (NULL);
    }
    ft_printf_fd(1, "Texture loaded: w %d, h %d, type %d\n",w,h,type);
    t_list *square_lst = cut_texture_into_squares(texture, w, h, 16, 16, type);
    if (!square_lst) {
        free(texture);
        ft_printf_fd(2, "Failed to cut texture\n");
        return (NULL);
    }
    ft_printf_fd(1, "Texture cuted\n");
    free(texture);

    GLuint *atlas = load_gl_texture_atlas(square_lst, type);
    lst_clear(&square_lst, free);
    return (atlas);
}
