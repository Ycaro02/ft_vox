#include "../../include/vox.h"			/* Main project header */
#include "../../include/chunks.h"		/* Main project header */
#include "../../include/render_chunks.h"	/* Main project header */

GLuint setupInstanceVBOForThisChunk(vec3* block_array, u32 visibleBlock) {
    return (bufferGlCreate(GL_ARRAY_BUFFER, visibleBlock * sizeof(vec3), (void *)block_array[0]));
}

void fillBlockArrayForChunk(RenderChunks *render, Chunks *chunks) {
    chunksCubeGet(chunks, render);
}

void renderChunkFree(RenderChunks *render) {
	free(render->blockTypeID);
	free(render->block_array);
	glDeleteBuffers(1, &render->instanceVBO);
	glDeleteBuffers(1, &render->typeBlockVBO);
	free(render);
}

void renderChunksMapFree(void *entry) {
	HashMap_entry *e = (HashMap_entry *)entry;
	if (e->value) {
		renderChunkFree((RenderChunks *)e->value);
	}
	free(e); /* free the entry t_list node */
}


RenderChunks *renderChunkCreate(HashMap *renderChunksCacheMap, Chunks *chunks) {
    RenderChunks *render = NULL;
	BlockPos	  chunkID;
	
	if (!chunks) {
		return (NULL);
	}
	
	chunkID = CHUNKS_MAP_ID_GET(chunks->x, chunks->z);

	/* Check if renderChunk is in renderChunksCache map */
	if ((render = hashmap_get(renderChunksCacheMap, chunkID))) {
		return (render);
	}


	if (!(render = malloc(sizeof(RenderChunks)))) {
		ft_printf_fd(2, "Failed to allocate render\n");
		return (NULL);
	}
	render->chunkID = chunkID;
    render->visibleBlock = chunks->visible_block;
	if (!(render->blockTypeID = ft_calloc(sizeof(f32), render->visibleBlock))) {
		ft_printf_fd(2, "Failed to allocate blockTypeID\n");
		free(render);
		return (NULL);
	} else if (!(render->block_array = ft_calloc(sizeof(vec3), render->visibleBlock))) {
		ft_printf_fd(2, "Failed to allocate block_array\n");
		free(render->blockTypeID);
		free(render);
		return (NULL);
	}
	fillBlockArrayForChunk(render, chunks);
	render->instanceVBO = setupInstanceVBOForThisChunk(render->block_array, render->visibleBlock); // crée un VBO pour les données d'instance de ce render
	render->typeBlockVBO = bufferGlCreate(GL_ARRAY_BUFFER, render->visibleBlock * sizeof(GLuint), (void *)&render->blockTypeID[0]);
	return (render);
}




HashMap *chunksToRenderChunks(Context *c, HashMap *chunksMap) {
    HashMap_it	it = hashmap_iterator(chunksMap);
    s8			next = 1; 

	HashMap *renderChunksMap = hashmap_init(HASHMAP_SIZE_1000, renderChunksMapFree);
    while ((next = hashmap_next(&it))) {
		BoundingBox box = chunkBoundingBoxGet((Chunks *)it.value, 8.0f, c->cam.position[1]);
		if (isChunkInFrustum(&c->cam.frustum, &box)) {
			RenderChunks *renderChunk = renderChunkCreate(c->world->renderChunksCacheMap, (Chunks *)it.value);
			hashmap_set_entry(renderChunksMap, RENDER_CHUNKS_ID(renderChunk), renderChunk);
		}
    }
    return (renderChunksMap);
}
