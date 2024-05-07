#include "include/perlin_noise.h"

int main (int argc, char **argv) {
	u64 seed = array_to_uint32(argv[1]);
	if (seed == OUT_OF_UINT32) {
		ft_printf_fd(2, RED"Error: Invalid seed\n"RESET);
		return (1);
	}

	randomGenerationInit(seed);
	int *array = randomMultipleGenerationGet(10, INT_MAX);
	for (int i = 0; i < 10; i++) {
		ft_printf_fd(1, ORANGE"%d, Rand: %d\n"RESET, i, array[i]);
	}
	free(array);


	vec2_f32 **noise = gradientNoiseGeneration(PERLIN_NOISE_WIDTH, PERLIN_NOISE_HEIGHT);
	if (!noise) {
		ft_printf_fd(2, RED"Error: Noise generation failed\n"RESET);
		return (1);
	}
	ft_printf_fd(1, GREEN"Success: Noise generation\n"RESET);
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; j++) {
			ft_printf_fd(1, "[%f][%f]\n, ", noise[i][j][0], noise[i][j][1]);
		}
		// ft_printf_fd(1, "\n");
	}

	free_incomplete_array((void **)noise, PERLIN_NOISE_HEIGHT);

	return 0;
}