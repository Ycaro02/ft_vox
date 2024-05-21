#include "../../include/vox.h"
#include "../../include/chunks.h"
#include "../../include/render_chunks.h"
#include "../../include/thread_load.h"

#define MAX_RENDER_DISTANCE 120.0f
#define TRAVEL_INCREMENT 6.0f

s8 chunksIsRenderer(HashMap *renderChunksMap, BlockPos chunkID) {
	return (hashmap_get(renderChunksMap, chunkID) != NULL);
}

s8 chunkIsLoaded(HashMap *chunksMap, BlockPos chunkID) {
	return (hashmap_get(chunksMap, chunkID) != NULL);
}

void worldToChunksPos(vec3 current, vec3 chunkOffset) {
    f32 chunkSize = 8.0; // cubeSize is 0.5
    chunkOffset[0] = floor(current[0] / chunkSize);
    chunkOffset[1] = floor(current[1] / chunkSize);
    chunkOffset[2] = floor(current[2] / chunkSize);
}

void renderChunksFrustrumRemove(Context *c, HashMap *renderChunksMap) {
	RenderChunks *render = NULL;
	HashMap_it it = hashmap_iterator(renderChunksMap);
	s8 next = TRUE;
	Chunks *chunks = NULL;

	while ((next = hashmap_next(&it))) {
		render = (RenderChunks *)it.value;
		BlockPos chunkID = render->chunkID;
		mtx_lock(&c->threadContext->mtx);
		chunks = hashmap_get(c->world->chunksMap, chunkID);
		mtx_unlock(&c->threadContext->mtx);
		if (chunks) {
			BoundingBox box = chunkBoundingBoxGet(chunks, 8.0f, c->cam.position[1]);
			if (!isChunkInFrustum(&c->cam.frustum, &box)) {
				hashmap_remove_entry(renderChunksMap, chunkID);
				/*	
					If we remove an entry we need to reset iterator 
					We can refactor this, with storing all key in a list and iterate over it
					at the end of the loop to remove all stored entry 
				*/
				it = hashmap_iterator(renderChunksMap);
			}
		}
	}
}

void chunksViewHandling(Context *c, HashMap *renderChunksMap) {
	RenderChunks *render = NULL;
    vec3 start_position, ray_direction, chunk_coords, current_position, travelVector;
    f32 current = 0;

    glm_vec3_copy(c->cam.position, start_position);
    glm_vec3_copy(c->cam.viewVector, ray_direction);
    glm_vec3_zero(chunk_coords);
    glm_vec3_zero(current_position);

	renderChunksFrustrumRemove(c, renderChunksMap);

	while ((current += TRAVEL_INCREMENT) <= MAX_RENDER_DISTANCE) {
		/* Scale ray dir */
		glm_vec3_scale(ray_direction, current, travelVector);
		/* add scaled ray vector add start position in current position */
		glm_vec3_add(start_position, travelVector, current_position);
		/* Convert world coordonate to chunk offset */
		worldToChunksPos(current_position, chunk_coords);

		BlockPos chunkID = {0, (s32)chunk_coords[0], (s32)chunk_coords[2]};
		mtx_lock(&c->threadContext->mtx);
		Chunks *chunks = hashmap_get(c->world->chunksMap, chunkID);
		mtx_unlock(&c->threadContext->mtx);

		s8 inView = 0;
		if (chunks) {
			BoundingBox box = chunkBoundingBoxGet(chunks, 8.0f, c->cam.position[1]);
			inView = isChunkInFrustum(&c->cam.frustum, &box);
		}

		if (!chunksIsRenderer(renderChunksMap, chunkID) && inView) {
			render = renderChunkCreate(chunks);
			hashmap_set_entry(renderChunksMap, chunkID, render);
		} 
	}

}
