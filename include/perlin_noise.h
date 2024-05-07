#ifndef HEADER_PERLIN_NOISE_H
#define HEADER_PERLIN_NOISE_H

#define PERLIN_NOISE_HEIGHT 256
#define PERLIN_NOISE_WIDTH 256

#include "../libft/libft.h"

FT_INLINE void randomGenerationInit(unsigned int seed) {
	srand(seed);
}

FT_INLINE int randomGenerationGet(int max) {
	return ((rand() - (max / 2)) % PERLIN_NOISE_HEIGHT);
}

FT_INLINE int *randomMultipleGenerationGet(int number, int max) {
	int *array = malloc(sizeof(int) * number);

	for (int i = 0; i < number; i++) {
		array[i] = ((rand() - (max / 2)) % PERLIN_NOISE_HEIGHT);
	}
	return (array);
}

FT_INLINE void noiseGradienCompute(int x, int y, vec2_f32 dest) {
	f64 powX = x * x;
	f64 powY = y * y;

	f64 distance = sqrt(powX + powY);

	dest[0] = x / distance;
	dest[1] = y / distance;
}

FT_INLINE vec2_f32 **gradientNoiseGeneration(int width, int height) {
	vec2_f32 **noise = malloc(sizeof(vec2_f32 *) * height);

	if (!noise) {
		return (NULL);
	}

	for (int i = 0; i < height; i++) {
		noise[i] = malloc(sizeof(vec2_f32 *) * width);
		if (!noise[i]) {
			free_incomplete_array((void **)noise, i);
			return (NULL);
		}
		for (int j = 0; j < width; j++) {
			noiseGradienCompute(randomGenerationGet(INT_MAX), randomGenerationGet(INT_MAX), noise[i][j]);
		}
	}
	return (noise);
}

#endif /* HEADER_PERLIN_NOISE_H */
