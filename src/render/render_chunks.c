#include "../../include/vox.h"			/* Main project header */
#include "../../include/world.h"		/* Main project header */
#include "../../include/chunks.h"		/* Main project header */
#include "../../include/render_chunks.h"	/* Main project header */


void renderChunkFree(RenderChunks *render) {
	free(render->faceCount);
	free(render->trspFaceCount);
	for (u8 i = 0; i < 6; ++i) {
		free(render->faceArray[i]);
		free(render->faceTypeID[i]);
		free(render->trspFaceArray[i]);
		free(render->trspTypeId[i]);
	}
	/* Care here we can't destroy VBO in sub thread */
	free(render);
}

void addRenderToVBOCreate(Context *c, BlockPos chunkID) {
	BlockPos	  *chunkIDVBOtoCreate = NULL;
	
	if ((chunkIDVBOtoCreate = malloc(sizeof(BlockPos)))) {
		ft_memcpy(chunkIDVBOtoCreate, &chunkID, sizeof(BlockPos));
		ft_lstadd_back(&c->vboToCreate, ft_lstnew(chunkIDVBOtoCreate));
	}
}

RenderChunks *renderChunkCreate(Context *c, Chunks *chunks) {
    RenderChunks *render = NULL;
	BlockPos	  chunkID;


	if (!chunks) {
		return (NULL);
	}
	
	chunkID = CHUNKS_MAP_ID_GET(chunks->x, chunks->z);
	if (!(render = ft_calloc(sizeof(RenderChunks), 1))) {
		ft_printf_fd(2, "Failed to allocate render\n");
		return (NULL);
	}
	render->chunkID = chunkID;
	chunksCubeFaceGet(&c->threadContext->chunkMtx, chunks, render);

	/* We need to store vbo to create in list to give it to main thread */
	addRenderToVBOCreate(c, chunkID);
	render->lastUpdate = get_ms_time();
	return (render);
}
