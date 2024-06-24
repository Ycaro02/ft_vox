#include "../include/vox.h"			/* Main project header */
#include "../include/skybox.h"		/* skybox rendering */
#include "../include/chunks.h"		/* Chunks */
#include "../include/render_chunks.h"
#include "../rsc/perlin_noise/include/perlin_noise.h"
#include "../include/thread_load.h"
#include "../include/cube.h"
#include "../include/world.h"
#include "../include/text_render.h"
#include "../include/camera.h"
#include "../include/shader_utils.h"
#include "../include/window.h"


u8 *perlinNoiseGeneration(unsigned int seed) {
	return (perlinImageGet(seed, PERLIN_NOISE_HEIGHT, PERLIN_NOISE_WIDTH
		, PERLIN_OCTAVE, PERLIN_PERSISTENCE, PERLIN_LACUNARITY));
	// return (perlinImageGet(seed, PERLIN_NOISE_HEIGHT, PERLIN_NOISE_WIDTH
	// 	, PERLIN_CONTINENTAL_OCTAVE, PERLIN_CONTINENTAL_PERSISTENCE, PERLIN_CONTINENTAL_LACUNARITY));
}

u8 *perlinNoiseGenerationWithoutSeed(s32 width, s32 height, s32 octaves, f32 persistence, f32 lacurarity) {
	return (perlinImageNoSeedGet(height, width, octaves
		, persistence, lacurarity));
}


void initSkyBox(Context *c) {
	c->skyboxVAO = skyboxInit();
	c->skyboxShaderID = load_shader(SKY_VERTEX_SHADER, SKY_FRAGMENT_SHADER);
	glUseProgram(c->skyboxShaderID);
	c->skyTexture = load_cubemap(TEXTURE_SKY_PATH, 1024, 1024);
	set_shader_texture(c->skyboxShaderID, c->skyTexture, GL_TEXTURE_CUBE_MAP, "texture1");

}

void initAtlasTexture(Context *c) {
	c->cubeShaderID = load_shader(CUBE_VERTEX_SHADER, CUBE_FRAGMENT_SHADER);
	c->blockAtlasId = load_texture_atlas(TEXTURE_ATLAS_PATH, 16, 16);
	set_shader_texture(c->cubeShaderID, c->blockAtlasId, GL_TEXTURE_3D, "textureAtlas");
}


f32 **perlin2DInit(u32 seed) {
	f32 **perlin2D = NULL;
	u8 *perlin1D = perlinNoiseGeneration(seed); /* seed 42 */
	if (!perlin1D) {
		ft_printf_fd(1, "Error: perlinNoise error\n");
		return (NULL);
	}
	/* Transform 1D array to 2D array */
	perlin2D = array1DTo2D(perlin1D, PERLIN_NOISE_HEIGHT, PERLIN_NOISE_WIDTH);
	free(perlin1D);
	return (perlin2D);
}


f32 **perlin2DGeneration(s32 octaves, f32 persistence, f32 lacunarity) {
	f32 **perlin2D = NULL;
	u8	*perlin1D = perlinNoiseGenerationWithoutSeed(PERLIN_NOISE_WIDTH, PERLIN_NOISE_HEIGHT
		, octaves, persistence, lacunarity); /* seed already generated */
	if (!perlin1D) {
		ft_printf_fd(1, "Error: perlinNoise error\n");
		return (NULL);
	}
	/* Transform 1D array to 2D array */
	perlin2D = array1DTo2D(perlin1D, PERLIN_NOISE_HEIGHT, PERLIN_NOISE_WIDTH);
	free(perlin1D);
	return (perlin2D);
}

u8	**perlinSnakeCave2DGet() {
	f32 **perlin2D = NULL;
	u8  **snakePerlin2D = NULL;
	u8 *perlin1D = perlinNoiseGenerationWithoutSeed(PERLIN_SNAKE_WIDTH, PERLIN_SNAKE_HEIGHT
		, PERLIN_SNAKE_OCTAVE, PERLIN_SNAKE_PERSISTENCE, PERLIN_SNAKE_LACUNARITY); /* seed 42 */
	if (!perlin1D) {
		ft_printf_fd(1, "Error: perlinNoise error\n");
		return (NULL);
	}
	/* Transform 1D array to 2D array */
	perlin2D = array1DTo2D(perlin1D, PERLIN_SNAKE_HEIGHT, PERLIN_SNAKE_WIDTH);
	free(perlin1D);
	snakePerlin2D = perlinToSnakeData(perlin2D, PERLIN_SNAKE_HEIGHT, PERLIN_SNAKE_WIDTH);
	for (u32 i = 0; i < PERLIN_SNAKE_HEIGHT; i++) {
		free(perlin2D[i]);
	}
	free(perlin2D);
	return (snakePerlin2D);
}

s8 mutexMultipleInit(Context *c) {

	if ( mtx_init(&c->renderMtx, mtx_plain) == thrd_error
		|| mtx_init(&c->gameMtx, mtx_plain) == thrd_error
		|| mtx_init(&c->isRunningMtx, mtx_plain) == thrd_error
		|| mtx_init(&c->vboToDestroyMtx, mtx_plain) == thrd_error
		|| mtx_init(&c->vboToCreateMtx, mtx_plain) == thrd_error
		|| mtx_init(&c->renderDataNeededMtx, mtx_plain) == thrd_error)
	{
		ft_printf_fd(1, "Error: mutex init failed\n");
		return (FALSE);
	}
	return (TRUE);
}

void scanNoiseGetMinMax(f32 **noise, f32 *min, f32 *max, vec2_u32 pos_min, vec2_u32 pos_max) {
	*min = noise[0][0];
	*max = noise[0][0];
	pos_min[0] = 0;
	pos_min[1] = 0;
	for (u32 i = 0; i < PERLIN_NOISE_HEIGHT; i++) {
		for (u32 j = 0; j < PERLIN_NOISE_WIDTH; j++) {
			if (noise[i][j] < *min) {
				*min = noise[i][j];
				pos_min[0] = i;
				pos_min[1] = j;
			}
			if (noise[i][j] > *max) {
				*max = noise[i][j];
				pos_max[0] = i;
				pos_max[1] = j;
			}
		}
	}
}

s8 multipleNoiseGeneration(Context *context, u32 seed) {

	if (!(context->world->noise.continental = perlin2DInit(seed))
		|| (!(context->world->noise.cave = perlinSnakeCave2DGet()))
		|| (!(context->world->noise.erosion = perlin2DGeneration(PERLIN_EROSION_OCTAVE, PERLIN_EROSION_PERSISTENCE, PERLIN_EROSION_LACUNARITY)))
		|| (!(context->world->noise.peaksValley = perlin2DGeneration(PERLIN_PICKS_VALLEY_OCTAVE, PERLIN_PICKS_VALLEY_PERSISTENCE, PERLIN_PICKS_VALLEY_LACUNARITY)))
		|| (!(context->world->noise.humidity = perlin2DGeneration(PERLIN_HUMIDITY_OCTAVE, PERLIN_HUMIDITY_PERSISTENCE, PERLIN_HUMIDITY_LACUNARITY)))
		|| (!(context->world->noise.temperature = perlin2DGeneration(PERLIN_TEMPERATURE_OCTAVE, PERLIN_TEMPERATURE_PERSISTENCE, PERLIN_TEMPERATURE_LACUNARITY)))
		)
	{
		ft_printf_fd(1, "Error: noise generation failed\n");
		return (FALSE);
	}
	f32 min, max;
	vec3_u32 min_pos = {0};
	vec3_u32 max_pos = {0};
	scanNoiseGetMinMax(context->world->noise.temperature, &min, &max, min_pos, max_pos);
	ft_printf_fd(1, RED"Temperature min: %f max: %f, min_pos: %u %u, max_pos %u %u\n"RESET,
	 min, max, min_pos[0], min_pos[1], max_pos[0], max_pos[1]);
	scanNoiseGetMinMax(context->world->noise.humidity, &min, &max, min_pos, max_pos);
	ft_printf_fd(1, YELLOW"Humidity min: %f max: %f\n"RESET, min, max);

	return (TRUE);
}

Context *contextInit() {
	Context *context;


	if (!(context = ft_calloc(sizeof(Context), 1))) {
		return (NULL);
	} else if (!mutexMultipleInit(context)) {
		return (NULL);
	}

	mtx_lock(&context->isRunningMtx);
	context->isPlaying = TRUE;
	mtx_unlock(&context->isRunningMtx);

	context->displayUndergroundBlock = TRUE;


	mtx_lock(&context->gameMtx);
	context->cam = create_camera(CAM_FOV, CAM_ASPECT_RATIO(SCREEN_WIDTH, SCREEN_HEIGHT), CAM_NEAR, CAM_FAR);
	extractFrustumPlanes(&context->cam->frustum, context->cam->projection, context->cam->view);
	mtx_unlock(&context->gameMtx);

	if (!(context->world = ft_calloc(sizeof(World), 1))
		|| (!((context->world->undergroundBlock = ft_calloc(sizeof(UndergroundBlock), 1))))
		|| (!(context->win_ptr = init_openGL_context()))
		|| (!(context->world->chunksMap = hashmap_init(HASHMAP_SIZE_2000, chunksMapFree)))
		|| (!(context->faceCube = cubeFaceVAOinit()))
		|| (!multipleNoiseGeneration(context, 42U)) /* Give seed here */
		|| (!(context->world->renderChunksMap = hashmap_init(HASHMAP_SIZE_1000, hashmap_free_node_only)))
		|| (!threadSupervisorInit(context))) 
	{
		return (NULL);
	} 

	/* init context camera */
    glm_mat4_identity(context->rotation);
	initSkyBox(context);
	initAtlasTexture(context);
	if (!freeTypeFontInit(context)) {
		return (NULL);
	}
	return (context);
}