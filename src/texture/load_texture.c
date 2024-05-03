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

void set_shader_texture(t_context *c, GLuint *atlas, u32 index)
{
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, atlas[index]);
    GLuint texture_loc = glGetUniformLocation(c->shader_id, "texture1");
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


GLuint *load_texture_atlas(char *path, int squareHeight, int squareWidth) {
    int w,h,type;

    u8 *texture = parse_bmp_file(path, &w, &h, &type);
    if (!texture) {
        ft_printf_fd(2, "Failed to load texture %s\n", path);
        return (NULL);
    }
    ft_printf_fd(1, "Texture loaded: w %d, h %d, type %d\n",w,h,type);
    t_list *square_lst = cut_texture_into_squares(texture, w, h, squareWidth, squareHeight, type);
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
