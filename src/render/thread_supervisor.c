#include "../../include/thread_load.h"
#include "../../include/chunks.h"
#include "../../include/render_chunks.h"

s8 chunkIsInQueue(HashMap *chunksMapToLoad, BlockPos chunkID) {
	return (hashmap_get(chunksMapToLoad, chunkID) != NULL);
}


Block *****chunkBlockCacheAlloc()
{
	Block *****cache = NULL;
	cache = malloc(sizeof(Block ****) * 16);
	if (!cache) {
		ft_printf_fd(2, "Error: chunkBlockCacheAlloc: malloc failed\n");
		return (NULL);
	}


	for (s32 subId = 0; subId < 16; ++subId) {
		cache[subId] = malloc(sizeof(Block ***) * 16);
		for (s32 x = 0; x < 16; ++x) {
			cache[subId][x] = malloc(sizeof(Block **) * 16);
			for (s32 y = 0; y < 16; ++y) {
				cache[subId][x][y] = malloc(sizeof(Block *) * 16);
				for (s32 z = 0; z < 16; ++z) {
					cache[subId][x][y][z] = NULL;
				}
			}
		}
	}
	return (cache);
}

void chunkBlockCacheFree(Block *****cache)
{
	for (s32 subId = 0; subId < 16; ++subId) {
		for (s32 x = 0; x < 16; ++x) {
			for (s32 y = 0; y < 16; ++y) {
				free(cache[subId][x][y]);
			}
			free(cache[subId][x]);
		}
		free(cache[subId]);
	}
}

/**
 * @brief Load chunks in a thread
 * @param data ThreadData
 * @return 1 (int function required by thrd_create)
*/
int threadChunksLoad(void *data) {

	/* Big block cache to avoid multiple call to hashmap_get for this chunk */
	Block 		*****chunksBlockCache = NULL;
	Chunks		*neighborChunksCache[4] = {NULL, NULL, NULL, NULL};
	ThreadData 	*t = (ThreadData *)data;
	Chunks 		*chunk = NULL;
	
	chunksBlockCache = chunkBlockCacheAlloc();
	if (!chunksBlockCache) {
		ft_printf_fd(2, "Error: threadChunksLoad: chunkBlockCacheAlloc failed\n");
		return (0);
	}

	chunk = chunksLoad(chunksBlockCache, t->c->perlin2D, t->chunkX, t->chunkZ, t->c->perlinCaveNoise);
	
	
	mtx_lock(t->chunkMtx);
	hashmap_set_entry(t->c->world->chunksMap, CHUNKS_MAP_ID_GET(t->chunkX, t->chunkZ), chunk);
	mtx_unlock(t->chunkMtx);
	
	chunkNeighborsGet(t->c, chunk, neighborChunksCache);

	mtx_lock(t->chunkMtx);
	updateChunkNeighbors(t->c, chunk, chunksBlockCache, neighborChunksCache);
	mtx_unlock(t->chunkMtx);

	chunkBlockCacheFree(chunksBlockCache);
	free(chunksBlockCache);

	return (1);
}


/**
 * @brief Wait for all the worker threads to finish their job
 * @param c Context
*/
void supervisorWaitWorker(Context *c) {
	s64 i = 0, max = 0;
	s32 status = 0;
	mtx_lock(&c->threadContext->threadMtx);
	max = c->threadContext->workerMax;
	mtx_unlock(&c->threadContext->threadMtx);

	while (i < max) {
		mtx_lock(&c->threadContext->threadMtx);
		status = c->threadContext->workers[i].busy;
		while (status == WORKER_BUSY) {
			mtx_unlock(&c->threadContext->threadMtx);
			usleep(10000);
			mtx_lock(&c->threadContext->threadMtx);
			status = c->threadContext->workers[i].busy;
		}
		mtx_unlock(&c->threadContext->threadMtx);
		ft_printf_fd(1, ORANGE"\nWorkers Thread: "RESET""YELLOW"[%d]"RESET""GREEN" finished: status %d"RESET, i, status);
		++i;
	}
}

/**
 * @brief Lock before to call this function
 * @brief Add chunks to the queue to load if not already in the queue
 * @param c Context
 * @param chunkX Chunk X
 * @param chunkZ Chunk Z
 * @return TRUE if success, FALSE if failed to add the chunks to the queue
*/
s8 chunksToLoadBuildMap(Context *c, s32 chunkX, s32 chunkZ) {
	ThreadData	*tdata = NULL;
	BlockPos 	chunkID = CHUNKS_MAP_ID_GET(chunkX, chunkZ);

	mtx_lock(&c->threadContext->chunkMtx);
	mtx_lock(&c->threadContext->threadMtx);
	if (chunkIsLoaded(c->world->chunksMap, chunkID)) {
		mtx_unlock(&c->threadContext->threadMtx);
		mtx_unlock(&c->threadContext->chunkMtx);
		return (TRUE);
	} 
	mtx_unlock(&c->threadContext->chunkMtx);
	
	if (chunkIsInQueue(c->threadContext->chunksMapToLoad, chunkID)\
		|| workerIsLoadingChunks(c, chunkX, chunkZ)) {
		mtx_unlock(&c->threadContext->threadMtx);
		return (TRUE);
	} else if (!(tdata = ft_calloc(sizeof(ThreadData), 1))) {
		mtx_unlock(&c->threadContext->threadMtx);
		return (FALSE);
	}

	tdata->c = c;
	tdata->chunkMtx = &c->threadContext->chunkMtx;
	tdata->chunkX = chunkX;
	tdata->chunkZ = chunkZ;

	BoundingBox box = {};
	box = chunkBoundingBoxGet(chunkX, chunkZ, 8.0f);
	if (!isChunkInFrustum(&c->gameMtx, &c->cam.frustum, &box)) {
		tdata->priority = LOAD_PRIORITY_LOW;
	} else {
		tdata->priority = LOAD_PRIORITY_HIGH;
	}

	hashmap_set_entry(c->threadContext->chunksMapToLoad, chunkID, tdata);
	mtx_unlock(&c->threadContext->threadMtx);
	return (TRUE);
}


/**
 * @brief Scan the environment to load chunks arround the camera
 * @param c Context pointer
 * @param curr_x Player's x position
 * @param curr_z Player's z position
 * @param radius The radius around the player to scan
*/
s8 supervisorLoadChunksArround(Context *c, s32 radius) {
	BlockPos 	pos = {0,0,0};
	s32 		currentX, currentZ;
	
	mtx_lock(&c->gameMtx);
	currentX = c->cam.chunkPos[0];
	currentZ = c->cam.chunkPos[2];
	mtx_unlock(&c->gameMtx);
	for (s32 i = -radius; i <= radius; ++i) {
		for (s32 j = -radius; j <= radius; ++j) {
			pos = CHUNKS_MAP_ID_GET(currentX + i, currentZ + j);
			if (!chunksToLoadBuildMap(c, pos.y, pos.z)) {
				ft_printf_fd(2, "Error: supervisorLoadChunksArround: chunksToLoadBuildMap failed\n");
				// mtx_unlock(&c->threadContext->chunkMtx);
				return (FALSE);
			}
		}
	}
	return (TRUE);
}

void chunksToLoadPrioritySet(Context *c, BlockPos chunkID, u8 priority) {
	ThreadData 		*tdata = NULL;

	mtx_lock(&c->threadContext->threadMtx);
	tdata = hashmap_get(c->threadContext->chunksMapToLoad, chunkID);
	if (tdata) {
		tdata->priority = priority;
	}
	mtx_unlock(&c->threadContext->threadMtx);
}

/** LOCK thread mtx before to call this function
 * @brief Get the nearest chunks to load from the queue
 * @param c Context
 * @param chunksMapToLoad HashMap of chunks to load
 * @return ThreadData pointer to the nearest chunks to load
*/
ThreadData *chunksToLoadNearestGet(Context *c, HashMap *chunksMapToLoad) {
	BlockPos 		pos = {0, 0, 0};
	HashMap_it		it = {};
	ThreadData		*tdata = NULL;
	ThreadData		*current = NULL;
	s32				distance = -1, tmpDistance = 0, camChunkX, camChunkZ;
	s8 				next = 0;
	u8				priotiry = LOAD_PRIORITY_LOW;

	mtx_lock(&c->gameMtx);
	camChunkX = c->cam.chunkPos[0];
	camChunkZ = c->cam.chunkPos[2];
	mtx_unlock(&c->gameMtx);

	it = hashmap_iterator(chunksMapToLoad);
	while ((next = hashmap_next(&it))) {
		pos = ((HashMap_entry *)it._current->content)->origin_data;
		current = it.value;
		tmpDistance = chunksEuclideanDistanceGet(camChunkX, camChunkZ, pos.y, pos.z);
		if (distance == -1 || (tmpDistance <= distance && current->priority >= priotiry) || current->priority > priotiry) {
			distance = tmpDistance;
			priotiry = current->priority;
			tdata = current;
		} 
	}
	if (!tdata) {
		return (NULL);
	}
	current = NULL;
	if (!(current = ft_calloc(sizeof(ThreadData), 1))) {
		return (NULL);
	}

	ft_memcpy(current, tdata, sizeof(ThreadData));
	hashmap_remove_entry(chunksMapToLoad, CHUNKS_MAP_ID_GET(tdata->chunkX, tdata->chunkZ), HASHMAP_FREE_DATA);
	return (current);
}

/**
 * @brief Thread supervisor function, manage the worker threads
 * @param context Context
 * @return TRUE if success, FALSE if failed
*/
s32 supervisorThreadRoutine(void *context) {
	Context		*c = (Context *)context;

	if (!threadWorkersInit(c)) {
		ft_printf_fd(2, "Error: threadWorkersInit failed\n");
		return (FALSE);
	}

	while (voxIsRunning(c)) {
		while (renderNeedDataGet(c)) {
			usleep(500);
		}
		supervisorLoadChunksArround(c, CHUNKS_LOAD_RADIUS);
	    renderChunksFrustrumRemove(c, c->world->renderChunksMap);
		unloadChunkHandler(c);
		chunksViewHandling(c);
		// chunksQueueRemoveHandling(c, &c->threadContext->threadMtx, &c->gameMtx, c->threadContext->chunksMapToLoad);
		// usleep(500);
	}

	supervisorWaitWorker(c);
	return (TRUE);
}

/**
 * @brief Initialize the thread supervisor
 * @param c Context
 * @return TRUE if success, FALSE if failed
*/
s8 threadSupervisorInit(Context *c) {
	c->threadContext = ft_calloc(sizeof(ThreadContext), 1);
	if (!c->threadContext) {
		ft_printf_fd(2, "Error: threadSupervisorInit: malloc failed\n");
		return (FALSE);
	}
	c->threadContext->chunksMapToLoad = hashmap_init(HASHMAP_SIZE_1000, hashmap_entry_free);
	if (!c->threadContext->chunksMapToLoad) {
		ft_printf_fd(2, "Error: threadSupervisorInit: hashmap_init failed\n");
		return (FALSE);
	}
	mtx_init(&c->threadContext->threadMtx, mtx_plain);
	mtx_init(&c->threadContext->chunkMtx, mtx_plain);
	mtx_init(&c->threadContext->logMtx, mtx_plain);
	thrd_create(&c->threadContext->supervisor, supervisorThreadRoutine, c);

	return (TRUE);
}


// void chunksQueueRemoveHandling(Context *c, Mutex *threadMtx, Mutex *gameMtx, HashMap *chunksMapToLoad) {
// 	HashMap_it it;
// 	s8 next = TRUE;
// 	BlockPos pos = {0, 0, 0};
// 	t_list *toRemoveList = NULL;
// 	BlockPos *chunkIDToRemove = NULL;
// 	s32 camChunkX, camChunkZ;

// 	mtx_lock(gameMtx);
// 	camChunkX = c->cam.chunkPos[0];
// 	camChunkZ = c->cam.chunkPos[2];
// 	mtx_unlock(gameMtx);

// 	mtx_lock(threadMtx); /* LOCK */
// 	it = hashmap_iterator(chunksMapToLoad);
// 	while ((next = hashmap_next(&it))) {
// 		pos = ((HashMap_entry *)it._current->content)->origin_data;
// 		if (chunksEuclideanDistanceGet(camChunkX, camChunkZ, pos.y, pos.z) > CHUNKS_UNLOAD_RADIUS) {
// 			if ((chunkIDToRemove = malloc(sizeof(BlockPos)))) {
// 				*chunkIDToRemove = pos;
// 				ft_lstadd_back(&toRemoveList, ft_lstnew(chunkIDToRemove));
// 			}
// 		}
// 	}

// 	for (t_list *current = toRemoveList; current; current = current->next) {
// 		hashmap_remove_entry(chunksMapToLoad, *(BlockPos *)current->content, HASHMAP_FREE_DATA);
// 	}
// 	ft_lstclear(&toRemoveList, free);
// 	mtx_unlock(threadMtx); /* UNLOCK */

// }