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
	free(render->block_array);
	free(render);
}

RenderChunks *renderChunkCreate(Chunks *chunks) {
    RenderChunks *render = NULL;
	
	if (!chunks) {
		// ft_printf_fd(2, "Chunks is NULL\n");
		return (NULL);
	}

	if (!(render = malloc(sizeof(RenderChunks)))) {
		ft_printf_fd(2, "Failed to allocate render\n");
		return (NULL);
	}
	render->chunkID = CHUNKS_MAP_ID_GET(chunks->x, chunks->z);
    render->visibleBlock = chunks->visible_block;
	if (!(render->blockTypeID = ft_calloc(sizeof(f32), render->visibleBlock))) {
		ft_printf_fd(2, "Failed to allocate blockTypeID\n");
		return (NULL);
	} else if (!(render->block_array = ft_calloc(sizeof(vec3), render->visibleBlock))) {
		ft_printf_fd(2, "Failed to allocate block_array\n");
		return (NULL);
	}
	fillBlockArrayForChunk(render, chunks);
	render->instanceVBO = setupInstanceVBOForThisChunk(render->block_array, render->visibleBlock); // crée un VBO pour les données d'instance de ce render
	render->typeBlockVBO = bufferGlCreate(GL_ARRAY_BUFFER, render->visibleBlock * sizeof(GLuint), (void *)&render->blockTypeID[0]);
	return (render);
}


/* Basic function you can provide to hashmap_init */
void renderChunksMapFree(void *entry) {
	HashMap_entry *e = (HashMap_entry *)entry;
	if (e->value) {
		RenderChunks *render = (RenderChunks *)e->value;
		free(render->blockTypeID);
		free(render->block_array);
		glDeleteBuffers(1, &render->instanceVBO);
		glDeleteBuffers(1, &render->typeBlockVBO);
		// close VBO
		free(e->value); /* free the value (alocated ptr content of t_list ) */
	}
	free(e); /* free the entry t_list node */
}


HashMap *chunksToRenderChunks(Context *c, HashMap *chunksMap) {
    HashMap_it	it = hashmap_iterator(chunksMap);
    s8			next = 1; 

	HashMap *renderChunksMap = hashmap_init(HASHMAP_SIZE_1000, renderChunksMapFree);
    while ((next = hashmap_next(&it))) {
		if (frustrumCheck(&c->cam, (Chunks *)it.value)) {
			RenderChunks *renderChunk = renderChunkCreate((Chunks *)it.value);
			hashmap_set_entry(renderChunksMap, RENDER_CHUNKS_ID(renderChunk), renderChunk);
		}
    }
    return (renderChunksMap);
}
