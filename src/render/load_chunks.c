#include "../../include/vox.h"
#include "../../include/chunks.h"
#include "../../include/render_chunks.h"
#include "../../include/thread_load.h"


/**
 * @brief Check if a chunk is already in render map
 * @param renderChunksMap Render chunks map
 * @param chunkID Chunk ID
 * @return 1 if chunk is already in render map, 0 otherwise
*/
s8 chunksIsRenderer(HashMap *renderChunksMap, BlockPos chunkID) {
	return (hashmap_get(renderChunksMap, chunkID) != NULL);
}

/**
 * @brief Check if a chunk is already in chunk map
 * @param renderChunksMap Render chunks map
 * @param chunkID Chunk ID
 * @return 1 if chunk is already in chunk map, 0 otherwise
*/
s8 chunkIsLoaded(HashMap *chunksMap, BlockPos chunkID) {
	return (hashmap_get(chunksMap, chunkID) != NULL);
}

/**
 * @brief Convert world position to chunk offset
 * @param current Current position
 * @param chunkOffset Chunk offset (output)
*/
void worldToChunksPos(vec3 current, vec3 chunkOffset) {
    f32 chunkSize = 8.0; // cubeSize is 0.5
    chunkOffset[0] = floor(current[0] / chunkSize);
    chunkOffset[1] = floor(current[1] / chunkSize);
    chunkOffset[2] = floor(current[2] / chunkSize);
}

/**
 * @brief Remove chunks from render map if not in frustum
 * @param c Context
 * @param renderChunksMap Render chunks map
*/
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
	RenderChunks	*render = NULL;
    vec3			start, rayDir, chunkPos, currPos, travelVector;
    f32				current = 0;

    glm_vec3_copy(c->cam.position, start);
    glm_vec3_copy(c->cam.viewVector, rayDir);
    glm_vec3_zero(chunkPos);
    glm_vec3_zero(currPos);


	while ((current += TRAVEL_INCREMENT) <= MAX_RENDER_DISTANCE) {
		/* Scale ray dir */
		glm_vec3_scale(rayDir, current, travelVector);
		/* add scaled ray vector add start position in current position */
		glm_vec3_add(start, travelVector, currPos);
		/* Convert world coordonate to chunk offset */
		worldToChunksPos(currPos, chunkPos);

		BlockPos chunkID = {0, (s32)chunkPos[0], (s32)chunkPos[2]};
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

	renderChunksFrustrumRemove(c, renderChunksMap);

}
