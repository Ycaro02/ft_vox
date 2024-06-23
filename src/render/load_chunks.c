
#include "../../include/vox.h"
#include "../../include/world.h"
#include "../../include/chunks.h"
#include "../../include/render_chunks.h"
#include "../../include/thread_load.h"
#include "../../include/camera.h"

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
 * @brief Check if a chunk render is alteady loaded
 * @param chunk Chunk
 * @return 1 if chunk render is loaded, 0 otherwise
*/
s8 chunksRenderIsLoaded(Chunks *chunk) {
	/* Must change name here to chunk render data is load != chunkrenderLoad (VBO loaded by main thread (instanceVBO != 0))*/
	if (chunk) {
		return (chunk->render != NULL);
	}
	return (FALSE);
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


void renderChunksVBODestroyListBuild(Context *c, Chunks *chunk) {
	GLuint		*instanceVBO = NULL, *typeBlockVBO = NULL;
	GLuint		*waterVBO = NULL, *waterTypeVBO = NULL;


	for (u32 i = 0; i < 6; ++i) {
		if ((instanceVBO = malloc(sizeof(GLuint))) && (typeBlockVBO = malloc(sizeof(GLuint)))) {
			*instanceVBO = chunk->render->faceVBO[i];
			*typeBlockVBO = chunk->render->faceTypeVBO[i];
			if (*instanceVBO != 0) {
				ft_lstadd_back(&c->vboToDestroy, ft_lstnew(instanceVBO));
			}
			if (*typeBlockVBO != 0) {
				ft_lstadd_back(&c->vboToDestroy, ft_lstnew(typeBlockVBO));
			}
		}
	}

	if ((waterVBO = malloc(sizeof(GLuint))) && (waterTypeVBO = malloc(sizeof(GLuint)))) {
		*waterVBO = chunk->render->topTransparencyFaceVBO;
		*waterTypeVBO = chunk->render->topTransparencyTypeVBO;
		if (*waterVBO != 0) {
			ft_lstadd_back(&c->vboToDestroy, ft_lstnew(waterVBO));
		}
		if (*waterTypeVBO != 0) {
			ft_lstadd_back(&c->vboToDestroy, ft_lstnew(waterTypeVBO));
		}
	}
}

void unloadChunkHandler(Context *c) {
	Chunks 		*chunk = NULL;
	HashMap_it 	it = {};
	// s8 			next = TRUE;
	t_list 		*toRemoveList = NULL;
	BlockPos 	*chunkIDToRemove = NULL;
	s32			camChunkX = 0, camChunkZ = 0;
	s32 		maxChunkLoad = CHUNKS_UNLOAD_RADIUS;

	mtx_lock(&c->gameMtx);
	camChunkX = c->cam->chunkPos[0];
	camChunkZ = c->cam->chunkPos[2];
	mtx_unlock(&c->gameMtx);

	mtx_lock(&c->vboToDestroyMtx);
	mtx_lock(&c->threadContext->chunkMtx);

	it = hashmap_iterator(c->world->chunksMap);
	while (hashmap_next(&it)) {
		chunk = (Chunks *)it.value;
		if (chunk) {
			s32 distance = chunksEuclideanDistanceGet(camChunkX, camChunkZ, chunk->x, chunk->z);
			BlockPos chunkID = CHUNKS_MAP_ID_GET(chunk->x, chunk->z);
			if (!chunksIsRenderer(c->world->renderChunksMap, chunkID) && distance > maxChunkLoad) {
				if ((chunkIDToRemove = malloc(sizeof(BlockPos)))) {
					ft_memcpy(chunkIDToRemove, &chunkID, sizeof(BlockPos));
					ft_lstadd_front(&toRemoveList, ft_lstnew(chunkIDToRemove));
					/* We need to store vbo to destroy in list to give it to main thread */
					if (chunk->render) {
						renderChunksVBODestroyListBuild(c, chunk);
					}
				}
			}
		}
	}


	for (t_list *current = toRemoveList; current; current = current->next) {
		hashmap_remove_entry(c->world->chunksMap, *(BlockPos *)current->content, HASHMAP_FREE_DATA);
	}

	mtx_unlock(&c->vboToDestroyMtx);
	mtx_unlock(&c->threadContext->chunkMtx);
	ft_lstclear(&toRemoveList, free);

}

/**
 * @brief Remove chunks from render map if not in frustum
 * @param c Context
 * @param renderChunksMap Render chunks map
*/
void renderChunksFrustrumRemove(Context *c, HashMap *renderChunksMap) {
	HashMap_it 		it;
	BlockPos 		tmpChunkID;
	t_list			*toRemoveList = NULL;
	BlockPos		*chunkIDToRemove = NULL;
	
	mtx_lock(&c->renderMtx);
	
	it = hashmap_iterator(renderChunksMap);
	while (hashmap_next(&it)) {
		BlockPos chunkID = ((RenderChunks *)it.value)->chunkID;
		BoundingBox box = chunkBoundingBoxGet(chunkID.y, chunkID.z, 8.0f);
		if (!isChunkInFrustum(&c->gameMtx, &c->cam->frustum, &box)) {
			if ((chunkIDToRemove = malloc(sizeof(BlockPos)))) {
				ft_memcpy(chunkIDToRemove, &chunkID, sizeof(BlockPos));
				ft_lstadd_front(&toRemoveList, ft_lstnew(chunkIDToRemove));
			}
		}
	}

	for (t_list *current = toRemoveList; current; current = current->next) {
		tmpChunkID = *(BlockPos *)current->content;
		hashmap_remove_entry(renderChunksMap, tmpChunkID, HASHMAP_FREE_NODE);
	}

	mtx_unlock(&c->renderMtx);

	ft_lstclear(&toRemoveList, free);

}

/**
 * @brief Handle chunks view, loop on camera fov and the distance to render chunks 
 * then check if chunk is in frustum and not already in render map to add it
 * @param c Context
 * @param renderChunksMap Render chunks map
*/
void chunksViewHandling(Context *c) {
	BoundingBox 	box;
    vec3            start, rayDir, chunkPos, currPos, travelVector, camViewVector;
	BlockPos 		chunkID;
	Chunks 			*chunk = NULL;
    f32             current = 0;
	f32 			radiusStart = (-CAM_FOV);
	f32 			radiusEnd = (CAM_FOV);
	// f32 			radiusStart = (-90.0f);
	// f32 			radiusEnd = (90.0f);
	s8 				inView = 0, chunksRenderIsload = 0, chunkInRenderMap = 0;
	u8				neightborChunkLoaded = 0;
	s8				renderVBOisLoaded = FALSE;

	mtx_lock(&c->gameMtx);
    glm_vec3_copy(c->cam->position, start);
	mtx_unlock(&c->gameMtx);
    
	glm_vec3_zero(chunkPos);
    glm_vec3_zero(currPos);


	/* Loop on the complete camera fov */
    for (f32 angle = radiusStart - ANGLE_INCREMENT; angle <= radiusEnd + ANGLE_INCREMENT; angle += ANGLE_INCREMENT) {
		
		mtx_lock(&c->gameMtx);
		glm_vec3_copy(c->cam->viewVector, camViewVector);
		mtx_unlock(&c->gameMtx);

        glm_vec3_copy(camViewVector, rayDir);
        glm_vec3_rotate(rayDir, glm_rad(angle), (vec3){0.0f, 1.0f, 0.0f});

        current = 0;
        while ((current += TRAVEL_INCREMENT) <= MAX_RENDER_DISTANCE) {
            /* Scale ray dir */
            glm_vec3_scale(rayDir, current, travelVector);
            /* add scaled ray vector add start position in current position */
            glm_vec3_add(start, travelVector, currPos);
            /* Convert world coordonate to chunk offset */
            worldToChunksPos(currPos, chunkPos);
            chunkID = CHUNKS_MAP_ID_GET(chunkPos[0], chunkPos[2]);

			mtx_lock(&c->threadContext->chunkMtx);
			/* Check if chunk is already loaded */
            chunk = hashmap_get(c->world->chunksMap, chunkID);
			if (chunk) {
                box = chunkBoundingBoxGet(chunk->x, chunk->z, 8.0f);
				chunksRenderIsload = chunksRenderIsLoaded(chunk);
                inView = isChunkInFrustum(&c->gameMtx, &c->cam->frustum, &box);
				chunkInRenderMap = chunksIsRenderer(c->world->renderChunksMap, chunkID);
				if (inView) { /* If chunk is in frustum */
					chunkNeighborMaskUpdate(c, chunk);
					neightborChunkLoaded = chunk->neighbors == CHUNKS_NEIGHBOR_LOADED;
					/* If chunk->render is not load and all nearby chunk are loaded (to wait full ocllusion culling)*/
					
					mtx_lock(&c->vboToCreateMtx);
					mtx_lock(&c->renderMtx);
					
					if (!chunksRenderIsload && neightborChunkLoaded) {
						chunk->render = renderChunkCreate(c, chunk);
						chunksRenderIsload = TRUE;
					} 
					/* If renderChunks is not in render map and he's completly loaded (VBO created in main thread) */
					if (!chunkInRenderMap && chunksRenderIsload) {
						renderVBOisLoaded = chunk->render->faceTypeVBO[5] != 0;
						if (renderVBOisLoaded) {
							hashmap_set_entry(c->world->renderChunksMap, chunkID, chunk->render);
						} 
					}
					mtx_unlock(&c->renderMtx);
					mtx_unlock(&c->vboToCreateMtx);
				}
            } else { /* If chunk not event load and is in frustrum */
				chunksToLoadPrioritySet(c, chunkID, LOAD_PRIORITY_HIGH);
			}
			mtx_unlock(&c->threadContext->chunkMtx);
		}
    }
}