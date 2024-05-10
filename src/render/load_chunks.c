#include "../../include/vox.h"
#include "../../include/chunks.h"
#include "../../include/render_chunks.h"

#define MAX_RENDER_DISTANCE 40.0f
#define TRAVEL_INCREMENT 6.0f

s8 chunksIsRenderer(HashMap *renderChunksMap, BlockPos chunkID) {
	return (hashmap_get(renderChunksMap, chunkID) != NULL);
}

s8 chunkIsLoaded(HashMap *chunksMap, BlockPos chunkID) {
	return (hashmap_get(chunksMap, chunkID) != NULL);
}

void worldToChunksPos(vec3 current, vec3 chunkOffset)
{
    f32 chunkSize = 8.0; // cubeSize is 0.5
    chunkOffset[0] = floor(current[0] / chunkSize);
    chunkOffset[1] = floor(current[1] / chunkSize);
    chunkOffset[2] = floor(current[2] / chunkSize);
}

void chunksViewHandling(Context *c, HashMap *renderChunksMap) {
    vec3 start_position, ray_direction, chunk_coords, current_position, travelVector;
    f32 current = 0;

    glm_vec3_copy(c->cam.position, start_position);
    glm_vec3_copy(c->cam.viewVector, ray_direction);
    glm_vec3_zero(chunk_coords);
    glm_vec3_zero(current_position);


	while ((current += TRAVEL_INCREMENT) <= MAX_RENDER_DISTANCE) {
		/* Scale ray dir */
		glm_vec3_scale(ray_direction, current, travelVector);
		/* add scaled ray vector add start position in current position */
		glm_vec3_add(start_position, travelVector, current_position);
		/* Convert world coordonate to chunk offset */
		worldToChunksPos(current_position, chunk_coords);

		BlockPos chunkID = {0, (s32)chunk_coords[0], (s32)chunk_coords[2]};
		if (!chunkIsLoaded(c->world->chunksMap, chunkID)) {
			Chunks *chunks = chunksLoad(c, chunkID.y, chunkID.z);
			hashmap_set_entry(c->world->chunksMap, chunkID, chunks);
		} else if (!chunksIsRenderer(renderChunksMap, chunkID)) {
			RenderChunks *render = renderChunkCreate(hashmap_get(c->world->chunksMap, chunkID));
			hashmap_set_entry(renderChunksMap, chunkID, render);
		}
	}
}
