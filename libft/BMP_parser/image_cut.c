#include "parse_bmp.h"

u8 **cut_texture_into_squares(u8* texture, int tex_width, int tex_height, int width, int height, int type) {
    int num_squares_x = tex_width / width;
    int num_squares_y = tex_height / height;
    int num_squares = num_squares_x * num_squares_y;

    u8 **all_square = ft_calloc(sizeof(u8*), (num_squares + 1));

    for (int i = 0; i < num_squares_y; i++) {
        for (int j = 0; j < num_squares_x; j++) {
            u8* square = malloc(width * height * type);  // type bytes per pixel (RGB or RGBA)
            if (!square) {
                ft_printf_fd(2, "Failed to allocate memory for image square\n");
                return (NULL);
            }

            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    int tex_index = ((i * height + y) * tex_width + (j * width + x)) * type;
                    int square_index = (y * width + x) * type;

                    ft_memcpy(&square[square_index], &texture[tex_index], type);
                }
            }
            all_square[i * num_squares_x + j] = square;
        }
    }
    ft_printf_fd(1, "Cut texture %d*%d into %d squares of %d\n", tex_width, tex_height, num_squares, width);
    return (all_square);
}