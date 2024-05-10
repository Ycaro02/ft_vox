#include "../include/vox.h"			/* Main project header */
#include "../include/skybox.h"		/* skybox rendering */
#include "../include/chunks.h"		/* Chunks */
#include "../include/render_chunks.h"
#include "../include/perlin_noise.h"


s8 chunksIsRenderer(HashMap *renderChunksMap, BlockPos chunkID) {
	return (hashmap_get(renderChunksMap, chunkID) != NULL);
}

s8 chunkIsLoaded(HashMap *chunksMap, BlockPos chunkID) {
	return (hashmap_get(chunksMap, chunkID) != NULL);
}

/* 
	We start to the local chunks position offset
	Then fire a ray from the camera to the view direction until max render distance
	Detect the chunk offset position of ray travel
	Then we can load the chunk
*/

#define MAX_RENDER_DISTANCE 36.0f

void drawLine(vec3 start, vec3 end) {
	glLineWidth(5.0f);
	glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex3fv(start);
	// ft_printf_fd(1, GREEN"Draw line from %f %f %f to %f %f %f\n"RESET, c->cam.position[0], c->cam.position[1], c->cam.position[2], end[0], end[1], end[2]);
    glVertex3fv(end);
    glEnd();
	glFlush();
}

void worldToChunksPos(vec3 current, vec3 chunkOffset)
{
    f32 chunkSize = 8.0; // cubeSize is 0.5
    chunkOffset[0] = floor(current[0] / chunkSize);
    chunkOffset[1] = floor(current[1] / chunkSize);
    chunkOffset[2] = floor(current[2] / chunkSize);
}

//void debugChunkView() {
//			if (i == 0 && current_travel_distance >= MAX_RENDER_DISTANCE) {
//				ft_printf_fd(1, PINK"\nFirst Ray: ");
//				ft_printf_fd(1, "Angle: %f ", angle);
//				ft_printf_fd(1, "Ray Direction: %f, %f, %f ", ray_direction[0], ray_direction[1], ray_direction[2]);
//				ft_printf_fd(1, "Current Position: %f, %f, %f ", current_position[0], current_position[1], current_position[2]);
//				ft_printf_fd(1, "Chunk Coords: %f, %f, %f ", chunk_coords[0], chunk_coords[1], chunk_coords[2]);
//				ft_printf_fd(1, "ChunkID: X:%d Z:%d\n"RESET, chunkID.y, chunkID.z);
//			} else if (i == num_rays - 1 && current_travel_distance >= MAX_RENDER_DISTANCE) {
//				ft_printf_fd(1, RED"Last Ray: ");
//				ft_printf_fd(1, "Angle: %f ", angle);
//				ft_printf_fd(1, "Ray Direction: %f, %f, %f ", ray_direction[0], ray_direction[1], ray_direction[2]);
//				ft_printf_fd(1, "Current Position: %f, %f, %f ", current_position[0], current_position[1], current_position[2]);
//				ft_printf_fd(1, "Chunk Coords: %f, %f, %f ", chunk_coords[0], chunk_coords[1], chunk_coords[2]);
//				ft_printf_fd(1, " -> LastChunkID: X:%d Z:%d\n"RESET, chunkID.y, chunkID.z);
//			}

//}

void chunksViewHandling(Context *c, HashMap *renderChunksMap) {
    vec3 start_position, ray_direction, chunk_coords, current_position;
    // f32 fov = 80.0f; // Angle de vue de la caméra

    glm_vec3_copy(c->cam.position, start_position);
    glm_vec3_copy(c->cam.viewVector, ray_direction); // Utilisez directement le viewVector de la caméra comme direction du rayon
    glm_vec3_zero(chunk_coords);
    glm_vec3_zero(current_position);

	/* try to extrapolate ray direction X and Z */
	// if (ray_direction[0] < 0.0f) {
	// 	ray_direction[0] = -1.0f;
	// } else {
	// 	ray_direction[0] = 1.0f;
	// }
	// if (ray_direction[2] < 0.0f) {
	// 	ray_direction[0] = -1.0f;
	// } else {
	// 	ray_direction[2] = 1.0f;
	// }


    // Distance de voyage maximale pour charger les chunks
    f32 max_travel_distance = MAX_RENDER_DISTANCE;

    // Pas de voyage entre chaque itération
    f32 travel_increment = 6.0f;

	f32 current_travel_distance = 0;
	vec3 travelVector;

	// Voyage jusqu'à la distance maximale
	while (current_travel_distance <= max_travel_distance) {
		// Mise à l'échelle de la direction du rayon par la distance de voyage actuelle
		glm_vec3_scale(ray_direction, current_travel_distance, travelVector);
		// Ajout du vecteur de direction du rayon à la position de départ
		glm_vec3_add(start_position, travelVector, current_position);
		// Conversion en coordonnées de chunk
		worldToChunksPos(current_position, chunk_coords);

		BlockPos chunkID = {0, (s32)chunk_coords[0], (s32)chunk_coords[2]};
		if (!chunkIsLoaded(c->world->chunksMap, chunkID)) {
			Chunks *chunks = chunksLoad(c, chunkID.y, chunkID.z);
			hashmap_set_entry(c->world->chunksMap, chunkID, chunks);
		} else if (!chunksIsRenderer(renderChunksMap, chunkID)) {
			RenderChunks *render = renderChunkCreate(hashmap_get(c->world->chunksMap, chunkID));
			hashmap_set_entry(renderChunksMap, chunkID, render);
		}

		// Augmenter la distance de voyage pour la prochaine itération
		current_travel_distance += travel_increment;
	}
}


void drawAllChunks(GLuint VAO, HashMap *renderChunksMap) {
	HashMap_it	it = hashmap_iterator(renderChunksMap);
	s8			next = 1;

	while ((next = hashmap_next(&it))) {
		drawAllCube(VAO, (RenderChunks *)it.value);
	}
}

void chunksRender(Context *c, GLuint VAO, GLuint shader_id, HashMap *renderChunksMap) {
	(void)c;
    glLoadIdentity();
	glUseProgram(shader_id);
	drawAllChunks(VAO, renderChunksMap);
    glFlush();
}


void vox_destroy(Context *c) {
	hashmap_destroy(c->world->chunksMap);
	free(c->world);
	free(c->cube.vertex);
    free(c->perlinNoise);
	glfwTerminate();
}

FT_INLINE void main_loop(Context *context, GLuint vao, GLuint skyTexture, HashMap *renderChunksMap) {
    while (!glfwWindowShouldClose(context->win_ptr)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		/* Input handling */
		glfwPollEvents();
        handle_input(context);
		
		/* Update data */
		update_camera(context, context->cubeShaderID);
        display_fps();
		chunksViewHandling(context, renderChunksMap);
		
		/* Render logic */
		// (void)vao, (void)skyTexture, (void)renderChunksMap;
        displaySkybox(context->skyboxVAO, skyTexture, context->skyboxShaderID, context->cam.projection, context->cam.view);
        chunksRender(context, vao, context->cubeShaderID, renderChunksMap);
	    
		// Appel à drawLine() avec des coordonnées fixes
		vec3 start_position;
		vec3 end_position = {0.0f, 0.0f, 0.0f};
		vec3 scaleView;
		glm_vec3_copy(context->cam.position, start_position); // Copie la position de la caméra dans start_position
		glm_vec3_scale(context->cam.viewVector, 20000.0f, scaleView);
		glm_vec3_add(start_position, scaleView, end_position); // Calcule la position finale du rayon
		// end_position[1] = context->cam.position[1];

		drawLine(start_position, end_position); // Dessine une ligne entre la position de départ et la position finale du rayon
		glfwSwapBuffers(context->win_ptr);
    }
}


/* Basic function you can provide to hashmap_init */
void chunksMapFree(void *entry) {
	HashMap_entry *e = (HashMap_entry *)entry;
	if (e->value) {
		Chunks *chunks = (Chunks *)e->value;
		for (u32 i = 0; chunks->sub_chunks[i].block_map ; ++i) {
			hashmap_destroy(chunks->sub_chunks[i].block_map);
		}
		free(e->value); /* free the value (allocaated ptr) */
	}
	free(e); /* free the entry t_list node */
}

u8 *perlinNoiseGeneration(unsigned int seed) {
	return (perlinImageGet(seed, PERLIN_NOISE_HEIGHT, PERLIN_NOISE_WIDTH, PERLIN_OCTAVE, PERLIN_PERSISTENCE, PERLIN_LACUNARITY));
}

int main() {
    Context context;
    GLFWwindow* window;

	ft_bzero(&context, sizeof(Context));
    window = init_openGL_context();
    context.win_ptr = window;

	if (!(context.world = ft_calloc(sizeof(World), 1))) {
		return (1);
	} else if (!(context.world->chunksMap = hashmap_init(HASHMAP_SIZE_100, chunksMapFree))) {
		return (1);
	}

	context.perlinNoise = perlinNoiseGeneration(42); /* seed 42 */
	if (!context.perlinNoise) {
		ft_printf_fd(1, "Error: perlinNoise error\n");
		return (1);
	}

	/* init context camera */
	context.cam = create_camera(80.0f, (float)(SCREEN_WIDTH / SCREEN_HEIGHT), 0.1f, 100.0f);
    glm_mat4_identity(context.cube.rotation);

	chunksLoadArround(&context, 1);
	GLuint cubeVAO = setupCubeVAO(&context, &context.cube);
	HashMap *renderChunksMap = chunksToRenderChunks(&context, context.world->chunksMap);

	/* Init skybox */
	context.skyboxVAO = skyboxInit();
	context.skyboxShaderID = load_shader(SKY_VERTEX_SHADER, SKY_FRAGMENT_SHADER);
	GLuint skyTexture = load_cubemap(TEXTURE_SKY_PATH, 1024, 1024);
	set_shader_texture(context.skyboxShaderID, skyTexture, GL_TEXTURE_CUBE_MAP, "texture1");

	/* Init cube */
	context.cubeShaderID = load_shader(CUBE_VERTEX_SHADER, CUBE_FRAGMENT_SHADER);
    GLuint textureAtlas = load_texture_atlas(TEXTURE_ATLAS_PATH, 16, 16);
	set_shader_texture(context.cubeShaderID, textureAtlas, GL_TEXTURE_3D, "textureAtlas");

	/* Disable VSync to avoid fps locking */
	// glfwSwapInterval(0);

	main_loop(&context, cubeVAO, skyTexture, renderChunksMap);

	// ft_lstclear(&renderChunksList, free);
	hashmap_destroy(renderChunksMap);
    vox_destroy(&context);
    return 0;
}
