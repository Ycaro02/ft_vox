#include "include/perlin_noise.h"

int main (int argc, char **argv) {
	u64 seed = array_to_uint32(argv[1]);
	if (seed == OUT_OF_UINT32) {
		ft_printf_fd(2, RED"Error: Invalid seed\n"RESET);
		return (1);
	}

	randomGenerationInit(seed);

	vec2_f32 **gradient = gradientNoiseGeneration(PERLIN_NOISE_WIDTH, PERLIN_NOISE_HEIGHT);
	if (!gradient) {
		ft_printf_fd(2, RED"Error: Noise generation failed\n"RESET);
		return (1);
	}
	ft_printf_fd(1, GREEN"Success: Noise generation\n"RESET);
	for (int i = 0; i < PERLIN_NOISE_HEIGHT; i++) {
		for (int j = 0; j < PERLIN_NOISE_WIDTH; j++) {
			ft_printf_fd(1, "[%f][%f]\n", gradient[i][j][0], gradient[i][j][1]);
		}
	}
	// free_incomplete_array((void **)gradient, PERLIN_NOISE_HEIGHT);

	for (int i = 0; i < PERLIN_NOISE_HEIGHT - 1; i++) {
		for (int j = 0; j < PERLIN_NOISE_WIDTH - 1; j++) {
			f32 float_i = (f32)i + 0.500;
			f32 float_j = (f32)j + 0.500; 
			f32 noise = perlinNoise(gradient, float_i, float_j);
			ft_printf_fd(1, YELLOW"for [%d][%d]:"RESET" "PINK"[%f]\n"RESET, i,j,noise);
		}
	}


	return (0);
}