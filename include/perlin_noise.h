#ifndef HEADER_PERLIN_NOISE_H
#define HEADER_PERLIN_NOISE_H

/* Height and width used for perlin noise generation */
#define PERLIN_NOISE_HEIGHT 1024
#define PERLIN_NOISE_WIDTH 1024

#define PERLIN_ARRAY_SIZE (PERLIN_NOISE_HEIGHT * PERLIN_NOISE_WIDTH)

#include "../libft/libft.h"

/**
 * @brief Initialize the random generation with a seed
 * must be called one time before any random generation
 * @param seed The seed to initialize the random generation
*/
FT_INLINE void randomGenerationInit(unsigned int seed) {
	srand(seed);
}

/**
 * @brief Get a random number between -max/2 and PERLIN_NOISE_HEIGHT
*/
FT_INLINE int randomGenerationGet() {
    f32 range = 1.0;
    f32 res = (f32)rand() / (f32)RAND_MAX * 2.0 * range - range;
	res *= 10000;
    return (res);
}

/**
 * noise_image.c *
*/
u8 *perlinImageGet(unsigned int seed, int width, int height, int octaves, f32 persistence, f32 lacurarity);

/* To call after seed generated */
u8 *perlinImageGetWithoutSeed(int width, int height, int octaves, f32 persistence, f32 lacurarity);

/**
 * perlin_noise.c *
*/

/**
 * @brief Generate a 2D array of gradient noise
 * @param width The width of the array
 * @param height The height of the array
 * @return vec2_f32** The 2D array of gradient noise
*/
vec2_f32 **gradientNoiseGeneration(int width, int height);


/**
 * @brief Compute the perlin noise value for x, y coordinates
 * @param gradient: Gradient vector
 * @param x, y: float coordinate
 * @return Perlin noise value
*/
f32 perlinNoise(vec2_f32 **gradient, f32 x, f32 y);

/**
 * @brief Generate a 2D sample of Perlin noise (Used in OctaveSample version)
 * @param width: Width of the sample
 * @param height: Height of the sample
 * @return 2D sample of Perlin noise
*/
f32 **noiseSample2D(vec2_f32 **gradient, int width, int height, f32 frequency);

/**
 * @brief Sample noise at a given octaves and frequency
 * @param gradient The 2D array of gradient noise
 * @param width The width of the array
 * @param height The height of the array
 * @param octaves The number of octaves to sample
 * @param frequency The frequency of the noise
 * @return f32** The 2D array of noise
 * @note The noise is sampled at the given frequency by scaling the coordinates
*/
f32 **perlinNoiseOctaveSample2D(vec2_f32 **gradient, int width, int height, int octaves, f32 persistence, f32 lacunarity);


/**
 * noise_utils.c *
*/

/**
 * @brief Get the minimum and maximum values in a 2D array
 * @param noise The 2D array of noise
 * @param min,max The minimum and maximum values in the array (input/output)
 * @param h,w The height and width of the array
*/
void minMaxNoiseGet(f32 **noise, f32 *min, f32 *max, int h, int w);

/**
 * @brief Normalize a value from one range to another
 * @param value The value to normalize
 * @param start1,stop1 The start and stop of the input range
 * @param start2,stop2 The start and stop of the output range
 * @return int The normalized value
*/
int normalize(f32 value, f32 start1, f32 stop1, int start2, int stop2);

/**
 * @brief Update the minimum and maximum color values
 * @param color The color to update
 * @param colorMin,colorMax The minimum and maximum color values (input/output)
*/
void debugColorUpdate(u8 color, u8 *colorMin, u8 *colorMax);

/**
 * @brief Update the minimum and maximum total values
 * @param total The total to update
 * @param totalMin,totalMax The minimum and maximum total values (input/output)
*/
void debugTotalUpdate(f32 total, f32 *totalMin, f32 *totalMax);

/**
 * @brief Allocate a 2D array of floats
 * @param rows,cols The number of rows and columns in the array
 * @return f32** The allocated 2D array
*/
f32 **floatDoubleArrayAlloc(int rows, int cols);

typedef struct s_perlin_data PerlinData;

f32 normalisef32Tof32(f32 value, f32 start1, f32 stop1, f32 start2, f32 stop2);
f32 perlinInterpolate(f32 a, f32 b, f32 t);
f32 bilinearInterpolation(f32 q11, f32 q12, f32 q21, f32 q22, f32 x, f32 z);
f32 getInterpolatedPerlinNoise(f32 **perlinNoise, f32 x, f32 z, f32 scale, s32 width, s32 height, PerlinData *perlinVal);
f32 normaliseNoiseGet(f32 **perlinNoise, s32 x, s32 z, PerlinData *perlinVal);

f32 **array1DTo2D(u8 *array, u32 height, u32 width);
f32 normalizeU8Tof32(u8 value, u8 start1, u8 stop1, f32 start2, f32 stop2);


/*
Sure, here's what each of these variables does in the context of Perlin noise:

`PERLIN_OCTAVE`: This defines the number of octaves to be used in the Perlin noise generation.
An octave in Perlin noise refers to a layer of noise. Each additional octave adds a finer-grained level of detail to the noise.
The higher the number of octaves, the more detailed and complex the resulting noise will be.
However, adding more octaves also increases the computation time.

`PERLIN_PERSISTENCE`: This controls the amplitude of each octave of noise.
It's usually a value between 0 and 1. A higher persistence value means that each octave
will contribute more to the final noise value, making the noise more detailed.
A lower persistence value means that each octave contributes less, making the noise smoother.
Persistence can be thought of as the roughness or choppiness of the noise.

`PERLIN_LACUNARITY`: This controls the frequency of each octave of noise.
The frequency determines how quickly the noise values change. A higher lacunarity value will make the noise change more rapidly,
resulting in a more "zoomed out" or "small-scale" noise. A lower lacunarity value will make the noise change more slowly,
resulting in a more "zoomed in" or "large-scale" noise.

These three variables together control the level of detail, roughness,
and scale of the Perlin noise. By adjusting these values, you can generate a wide variety of noise patterns.
*/

/* mlx init for display */
//int8_t init_mlx(int width, int height, u8 **perlinData);

#endif /* HEADER_PERLIN_NOISE_H */
