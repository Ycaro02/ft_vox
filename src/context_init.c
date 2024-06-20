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
}

u8 *perlinNoiseGenerationWithoutSeed(s32 width, s32 height, s32 octaves, f32 persistence, f32 lacurarity) {
	return (perlinImageNoSeedGet(height, width, octaves
		, persistence, lacurarity));
}


void resetOpenGLContext() {
	glUseProgram(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0); // Pour les textures 2D
}

void initSkyBox(Context *c) {
	c->skyboxVAO = skyboxInit();
	c->skyboxShaderID = load_shader(SKY_VERTEX_SHADER, SKY_FRAGMENT_SHADER);
	glUseProgram(c->skyboxShaderID);
	c->skyTexture = load_cubemap(TEXTURE_SKY_PATH, 1024, 1024);
	set_shader_texture(c->skyboxShaderID, c->skyTexture, GL_TEXTURE_CUBE_MAP, "texture1");

}


GLuint test_shader(char *vertexShader, char *fragmentShader)
{
	char *vertex_shader = load_shader_file(vertexShader);
	char *fragment_shader = load_shader_file(fragmentShader);
	
	/* create shader */
	GLuint frag_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	GLuint frag_pixel_shader = glCreateShader(GL_FRAGMENT_SHADER);

	/* compile shader */
	glShaderSource(frag_vertex_shader, 1, (const char **)&vertex_shader, NULL);
	glCompileShader(frag_vertex_shader);

	glShaderSource(frag_pixel_shader, 1, (const char **)&fragment_shader, NULL);
	glCompileShader(frag_pixel_shader);

	GLuint shaderID = glCreateProgram();
	
	/* Attach and link shader program  */
	glAttachShader(shaderID , frag_vertex_shader);
	glAttachShader(shaderID , frag_pixel_shader);
	
	glLinkProgram(shaderID);

	GLint succes = 0;
	glGetProgramiv(shaderID , GL_LINK_STATUS, &succes);
	if (!succes) {
		GLchar data[1024];
		ft_bzero(data, 1024);
		glGetProgramInfoLog(shaderID , 512, NULL, data);
		ft_printf_fd(2, "Shader program log: %s\n", data);
	} 

	// glUseProgram(shaderID);

	/* delete shader fragment */
	glDeleteShader(frag_vertex_shader);
	glDeleteShader(frag_pixel_shader);

	/* delete ressource */
	free(vertex_shader);
	free(fragment_shader);
	// return (0);
	return (shaderID);
}

void initAtlasTexture(Context *c) {
	c->cubeShaderID = test_shader(CUBE_VERTEX_SHADER, CUBE_FRAGMENT_SHADER);
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
		|| (!(context->world->noise.continental = perlin2DInit(42U)))
		|| (!(context->world->noise.cave = perlinSnakeCave2DGet()))
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