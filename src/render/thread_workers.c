#include "../../include/thread_load.h"
#include "../../include/chunks.h"
#include "../../include/render_chunks.h"

s32 workersThreadRoutine(void *data) {
	ThreadData *inputData = (ThreadData *)data;
	ThreadData *tmp = NULL;

	// mtx_lock(&inputData->c->threadContext->threadMtx);
	// inputData->c->threadContext->workers[inputData->threadID].busy = WORKER_BUSY;
	// mtx_unlock(&inputData->c->threadContext->threadMtx);

	while (1) {
		while (renderNeedDataGet(inputData->c)) {
			usleep(500);
		}

		mtx_lock(&inputData->c->threadContext->threadMtx);
		tmp = chunksToLoadNearestGet(inputData->c, inputData->c->threadContext->chunksMapToLoad);
		if (tmp) {
			inputData->c->threadContext->workers[inputData->threadID].busy = WORKER_BUSY;
			inputData->chunkX = tmp->chunkX;
			inputData->chunkZ = tmp->chunkZ;
			inputData->priority = tmp->priority;
			// VOX_PROTECTED_LOG(inputData->c, ORANGE"Thread %d: Loading chunk %d %d\n"RESET, inputData->threadID, tmp->chunkX, tmp->chunkZ);
			mtx_unlock(&inputData->c->threadContext->threadMtx);
			free(tmp);
			threadChunksLoad(inputData);
			
		} else {
			mtx_unlock(&inputData->c->threadContext->threadMtx);
			usleep(10000);
			if (!voxIsRunning(inputData->c)) {
				break;
			}
		}
	}

	mtx_lock(&inputData->c->threadContext->threadMtx);
	inputData->c->threadContext->workers[inputData->threadID].busy = WORKER_FREE;
	mtx_unlock(&inputData->c->threadContext->threadMtx);
	return (1);
}


/**
 * @brief Initialize the thread workers system, allocate memory for the workers based 
 * on the number of threads available on the system
 * @param c Context
 * @return 1 if success, 0 if failed to init the workers threads
*/
s8 threadWorkersInit(Context *c) {

	c->threadContext->workerMax = ThreadsAvailableGet();
	c->threadContext->workers = ft_calloc(sizeof(ThreadEntity), c->threadContext->workerMax);
	if (!c->threadContext->workers) {
		ft_printf_fd(2, "Error: threadWorkersInit: malloc failed\n");
		return (FALSE);
	}
	VOX_PROTECTED_LOG(c, YELLOW"Supervisor Thread Init: %d workers\n"RESET, (s32)c->threadContext->workerMax);

	/* TO move in is own func thread worker load */
	mtx_lock(&c->threadContext->threadMtx);
	for (s32 i = 0; i < c->threadContext->workerMax; ++i) {
		c->threadContext->workers[i].busy = WORKER_FREE;
		if (!(c->threadContext->workers[i].data = ft_calloc(sizeof(ThreadData), 1))) {
			VOX_PROTECTED_LOG(c, "Error: threadWorkersInit: malloc failed\n");
			return (FALSE);
		}
		c->threadContext->workers[i].data->c = c;
		c->threadContext->workers[i].data->chunkMtx = &c->threadContext->chunkMtx;
		c->threadContext->workers[i].data->threadID = i;
		c->threadContext->workers[i].data->priority = LOAD_PRIORITY_LOW;
		VOX_PROTECTED_LOG(c, ORANGE"Worker Thread Init: %d\n"RESET, i);
		thrd_create(&c->threadContext->workers[i].thread, workersThreadRoutine, c->threadContext->workers[i].data);
		if (!c->threadContext->workers[i].thread) {
			VOX_PROTECTED_LOG(c, "Error: threadWorkersInit: thrd_create failed\n");
			return (FALSE);
		}
		if (!thrd_detach(c->threadContext->workers[i].thread)) {
			VOX_PROTECTED_LOG(c, "Error: threadWorkersInit: thrd_detach failed\n");
			return (FALSE);
		}
	}
	mtx_unlock(&c->threadContext->threadMtx);

	return (TRUE);
}


/** 
 * @brief Lock before to call this function
 * @brief Check if a worker is already loading the given chunks
 * @param c Context
 * @param chunkX Chunk X
 * @param chunkZ Chunk Z
 * @return TRUE if a worker is already loading the chunks FALSE otherwise
*/
s8 workerIsLoadingChunks (Context *c, s32 chunkX, s32 chunkZ) {
	for (s32 i = 0; i < c->threadContext->workerMax; ++i) {
		if (c->threadContext->workers[i].busy == WORKER_BUSY
			&& c->threadContext->workers[i].data->chunkX == chunkX
			&& c->threadContext->workers[i].data->chunkZ == chunkZ) {
			return (TRUE);
		}
	}
	return (FALSE);
}

