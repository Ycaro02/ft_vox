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
	/* Care here we can't destroy VBO in sub thread */
	free(render);
}

void renderChunksMapFree(void *entry) {
	HashMap_entry *e = (HashMap_entry *)entry;
	if (e->value) {
		renderChunkFree((RenderChunks *)e->value);
	}
	free(e); /* free the entry t_list node */
}


RenderChunks *renderChunkCreateVBO(Mutex *chunkMtx, HashMap *chunksMap, BlockPos chunkID) {
	/* Create VBO */
	Chunks 			*chunks = NULL; 
	RenderChunks 	*render = NULL; 


	mtx_lock(chunkMtx);
	chunks = hashmap_get(chunksMap, chunkID);
	
	if (chunks) {
		render = chunks->render;
		if (render) {
			render->instanceVBO = setupInstanceVBOForThisChunk(render->block_array, render->visibleBlock);
			render->typeBlockVBO = bufferGlCreate(GL_ARRAY_BUFFER, render->visibleBlock * sizeof(GLuint), (void *)&render->blockTypeID[0]);
		}
	}
	mtx_unlock(chunkMtx);
	return (render);
}

RenderChunks *renderChunkCreate(Context *c, Chunks *chunks) {
    RenderChunks *render = NULL;
	BlockPos	  chunkID;
	BlockPos	  *chunkIDVBOtoCreate = NULL;


	if (!chunks) {
		return (NULL);
	}
	
	chunkID = CHUNKS_MAP_ID_GET(chunks->x, chunks->z);

	if (!(render = ft_calloc(sizeof(RenderChunks), 1))) {
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

	/* We need to store vbo to create in list to give it to main thread */
	if ((chunkIDVBOtoCreate = malloc(sizeof(BlockPos)))) {
		ft_memcpy(chunkIDVBOtoCreate, &chunkID, sizeof(BlockPos));
		ft_lstadd_back(&c->vboToCreate, ft_lstnew(chunkIDVBOtoCreate));
	}

	(void)c;
	/*	
		Instead of create VBO here we can build a list of BlockPos,
		represent render chunk just created
		On main thread iter on this list an create vbo
	*/
	// renderChunkCreateVBO(&c->threadContext->chunkMtx, c->world->chunksMap, chunkID);
	// render->instanceVBO = setupInstanceVBOForThisChunk(render->block_array, render->visibleBlock);
	// render->typeBlockVBO = bufferGlCreate(GL_ARRAY_BUFFER, render->visibleBlock * sizeof(GLuint), (void *)&render->blockTypeID[0]);
	
	return (render);
}
