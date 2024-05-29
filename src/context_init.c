#include "../include/vox.h"			/* Main project header */
#include "../include/skybox.h"		/* skybox rendering */
#include "../include/chunks.h"		/* Chunks */
#include "../include/render_chunks.h"
#include "../include/perlin_noise.h"
#include "../include/thread_load.h"


f32 normalizeU8Tof32(u8 value, u8 start1, u8 stop1, f32 start2, f32 stop2) {
    return start2 + (stop2 - start2) * ((value - start1) / (f32)(stop1 - start1));
}

f32 **array1DTo2D(u8 *array, u32 height, u32 width) {
	f32 **perlin2D = ft_calloc(height, sizeof(f32 *));
	for (u32 i = 0; i < height; ++i) {
		perlin2D[i] = ft_calloc(width, sizeof(f32));
		for (u32 j = 0; j < width; ++j) {
			perlin2D[i][j] = normalizeU8Tof32(array[i * width + j], 0, 255, -1.0f, 1.0f);
		}
	}
	return (perlin2D);
}

u8 *perlinNoiseGeneration(unsigned int seed) {
	return (perlinImageGet(seed, PERLIN_NOISE_HEIGHT, PERLIN_NOISE_WIDTH, PERLIN_OCTAVE, PERLIN_PERSISTENCE, PERLIN_LACUNARITY));
}

void initSkyBox(Context *c) {
	c->skyboxVAO = skyboxInit();
	c->skyboxShaderID = load_shader(SKY_VERTEX_SHADER, SKY_FRAGMENT_SHADER);
	c->skyTexture = load_cubemap(TEXTURE_SKY_PATH, 1024, 1024);
	set_shader_texture(c->skyboxShaderID, c->skyTexture, GL_TEXTURE_CUBE_MAP, "texture1");
}

void initAtlasTexture(Context *c) {
	c->cubeShaderID = load_shader(CUBE_VERTEX_SHADER, CUBE_FRAGMENT_SHADER);
	GLuint textureAtlas = load_texture_atlas(TEXTURE_ATLAS_PATH, 16, 16);
	set_shader_texture(c->cubeShaderID, textureAtlas, GL_TEXTURE_3D, "textureAtlas");
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

Context *contextInit() {
	Context *context;


	if (!(context = ft_calloc(sizeof(Context), 1))) {
		return (NULL);
	}

	mtx_init(&context->renderMtx, mtx_plain);
	mtx_init(&context->gameMtx, mtx_plain);

	mtx_lock(&context->gameMtx);
	context->isPlaying = TRUE;
	context->cam = create_camera(CAM_FOV, CAM_ASPECT_RATIO(SCREEN_WIDTH, SCREEN_HEIGHT), CAM_NEAR, CAM_FAR);
	mtx_unlock(&context->gameMtx);
	extractFrustumPlanes(&context->gameMtx ,&context->cam.frustum, context->cam.projection, context->cam.view);

	// || (!(context->cubeVAO = setupCubeVAO(&context->cube)))

	if (!(context->world = ft_calloc(sizeof(World), 1))
		|| (!(context->win_ptr = init_openGL_context()))
		|| (!(context->world->chunksMap = hashmap_init(HASHMAP_SIZE_2000, chunksMapFree)))
		|| (!(context->faceCube = cubeFaceVAOinit()))
		|| (!(context->perlin2D = perlin2DInit(42U)))
		|| (!(context->world->renderChunksMap = hashmap_init(HASHMAP_SIZE_2000, hashmap_free_node_only)))
		|| (!threadSupervisorInit(context))) 
	{
		return (NULL);
	} 

	/* init context camera */
    glm_mat4_identity(context->rotation);


	initSkyBox(context);
	initAtlasTexture(context);
	return (context);
}