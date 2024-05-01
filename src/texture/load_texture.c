#include "../../include/vox.h"

t_list *load_texture_atlas() {
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
    return (square_lst);
}
