#include "parse_bmp.h"

s8 is_square_pink(u8* square, int width, int height, int type) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int square_index = (y * width + x) * type;

            // If the pixel is not pink (RGB: 255,0,255 or RGBA: 255,0,255,255), return false
            if (!(square[square_index] == 255 && square[square_index + 1] == 0 && square[square_index + 2] == 255)) {
                return (FALSE);
            }
        }
    }
    // If all pixels were pink, return true
    return (TRUE);
}

t_list *cut_texture_into_squares(u8* texture, int tex_width, int tex_height, int width, int height, int type) {
    int num_squares_x = tex_width / width;
    int num_squares_y = tex_height / height;
    int square_count = 0;
    t_list *square_lst = NULL;

    for (int i = 0; i < num_squares_y; i++) {
        for (int j = 0; j < num_squares_x; j++) {
            u8* square = malloc(width * height * type);  /* type bytes per pixel (RGB or RGBA) */
            if (!square) {
                ft_printf_fd(2, "Failed to allocate memory for image square\n");
                return (NULL);
            }

            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    int tex_index = ((i * height + y) * tex_width + (j * width + x)) * type;
                    int square_index = (y * width + x) * type;

                    ft_memcpy(&square[square_index], &texture[tex_index], type);

                    /* If the square is pink (RGB: 255,0,255 or RGBA: 255,0,255,255), skip this square */
                    if (is_square_pink(square, width, height, type)) {
                        free(square);
                        square = NULL;
                        break;
                    }
                }

                if (!square) break;  // if square was skipped, break from the y loop as well
            }

            if (square) {
                // all_square[square_count] = square;
                ft_lstadd_back(&square_lst, ft_lstnew(square));
                square_count++;
            }
        }
    }
    ft_printf_fd(1, "Cut texture %d*%d into %d squares of %d\n", tex_width, tex_height, square_count, width);
    return (square_lst);

}