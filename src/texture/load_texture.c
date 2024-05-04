#include "../../include/vox.h"

void init_openGL_texture(u8 *data, u32 width, u32 height, u16 texture_type, GLuint* textures, int index)
{
    glGenTextures(1, &textures[index]);
    glBindTexture(GL_TEXTURE_2D, textures[index]);
    /* Set the texture wrapping parameters */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    /* Set texture filtering parameters */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);    // Set texture filtering to GL_LINEAR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    /* Load the texture */
    glTexImage2D(GL_TEXTURE_2D, 0, texture_type, width, height, 0, texture_type, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

}

void set_shader_texture(GLuint shaderId, GLuint *atlas, u32 index, u32 textureType)
{
	glUseProgram(shaderId);
    glActiveTexture(GL_TEXTURE0 + index);
    // glBindTexture(GL_TEXTURE_2D, atlas[index]);
    glBindTexture(textureType, atlas[index]);
    GLuint texture_loc = glGetUniformLocation(shaderId, "texture1");
    glUniform1i(texture_loc, index);
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
        init_openGL_texture(data, 16, 16, data_type, texture_id, i);
        ++i;
    }
    return (texture_id);
}


GLuint *load_texture_atlas(char *path, int squareHeight, int squareWidth, vec3_u8 ignore_color) {
    int w,h,type;

    u8 *texture = parse_bmp_file(path, &w, &h, &type);
    if (!texture) {
        ft_printf_fd(2, "Failed to load texture %s\n", path);
        return (NULL);
    }
    ft_printf_fd(1, "Texture loaded: w %d, h %d, type %d\n",w,h,type);
    t_list *square_lst = cut_texture_into_squares(texture, w, h, squareWidth, squareHeight, type, ignore_color);
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

GLuint load_cubemap(char* path, int squareHeight, int squareWidth, vec3_u8 ignore_color) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int w, h, type;
    u8 *texture = parse_bmp_file(path, &w, &h, &type);
    if (!texture) {
        ft_printf_fd(2, "Failed to load texture %s\n", path);
        return 0;
    }
    ft_printf_fd(1, "Texture loaded: w %d, h %d, type %d\n", w, h, type);

	u8 *fliped_text = imageFlip180(texture, w, h, type);
	free(texture);

    t_list *square_lst = cut_texture_into_squares(fliped_text, w, h, squareWidth, squareHeight, type, ignore_color);
    if (!square_lst) {
        free(fliped_text);
        ft_printf_fd(2, "Failed to cut fliped_text\n");
        return 0;
    }
    free(fliped_text);

    // t_list *current = square_lst;
	u32 data_type = type == 3 ? GL_RGB : GL_RGBA;

	/* X face */
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 0, 0, data_type, squareWidth, squareHeight, 0,
		data_type, GL_UNSIGNED_BYTE, (u8 *)get_lst_index_content(square_lst, 2));
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 1, 0, data_type, squareWidth, squareHeight, 0,
		data_type, GL_UNSIGNED_BYTE, (u8 *)get_lst_index_content(square_lst, 4));

	/* Y face */
    u8 *top_face = get_lst_index_content(square_lst, 0);
    u8 *flip_topface = imageFlip180(top_face, squareWidth, squareHeight, type);

	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 2, 0, data_type, squareWidth, squareHeight, 0,
		data_type, GL_UNSIGNED_BYTE, flip_topface);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 3, 0, data_type, squareWidth, squareHeight, 0,
		data_type, GL_UNSIGNED_BYTE, get_lst_index_content(square_lst, 5));
    free(flip_topface);

	/* Z face */
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 4, 0, data_type, squareWidth, squareHeight, 0,
		data_type, GL_UNSIGNED_BYTE, get_lst_index_content(square_lst, 1));
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 5, 0, data_type, squareWidth, squareHeight, 0,
		data_type, GL_UNSIGNED_BYTE, get_lst_index_content(square_lst, 3));

    lst_clear(&square_lst, free);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return (textureID);
}