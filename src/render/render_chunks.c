#include "../../include/vox.h"			/* Main project header */
#include "../../include/chunks.h"
#include "../../include/render_chunks.h"

GLuint setupInstanceVBOForThisChunk(vec3* block_array, u32 visibleBlock) {
    return (bufferGlCreate(GL_ARRAY_BUFFER, visibleBlock * sizeof(vec3), (void *)block_array[0]));
}

void fillBlockArrayForChunk(RenderChunks *render, Chunks *chunks) {
    chunks_cube_get(chunks, render->block_array, 0);
}


void renderChunkFree(RenderChunks *render) {
	free(render->block_array);
	free(render);
}

RenderChunks *renderChunkCreate(Chunks *chunks) {
    RenderChunks *render = malloc(sizeof(RenderChunks));
	if (!render) {
		ft_printf_fd(2, "Failed to allocate render\n");
		return (NULL);
	}
    render->visibleBlock = chunks->visible_block;
    render->block_array = ft_calloc(sizeof(vec3), render->visibleBlock);
    if (!render->block_array) {
		ft_printf_fd(2, "Failed to allocate block_array\n");
		return (NULL);
	}
	fillBlockArrayForChunk(render, chunks);
	render->instanceVBO = setupInstanceVBOForThisChunk(render->block_array, render->visibleBlock); // crée un VBO pour les données d'instance de ce render
	return (render);
}


t_list *chunksToRenderChunks(Context *c, HashMap *chunksMap) {
	t_list *renderChunksList = NULL;
    HashMap_it it = hashmap_iterator(chunksMap);
    s8 next = hashmap_next(&it); 

	(void)c;
    while (next) {
        Chunks *chunks = (Chunks *)it.value;
        RenderChunks *renderChunk = renderChunkCreate(chunks);
        ft_lstadd_back(&renderChunksList, ft_lstnew(renderChunk));
        next = hashmap_next(&it);
    }

    return (renderChunksList);
}