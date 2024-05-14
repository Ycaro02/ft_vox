#include "../../include/thread_load.h"
#include "../../include/chunks.h"


/**
 * @brief Initialize the thread workers system, allocate memory for the workers based 
 * on the number of threads available on the system
 * @param c Context
 * @return 1 if success, 0 if failed to init the workers threads
*/
s8 threadWorkersInit(Context *c) {

	c->threadContext->workerMax = ThreadsAvailableGet();
	c->threadContext->workerCurrent = 0;
	c->threadContext->workers = ft_calloc(sizeof(ThreadEntity), c->threadContext->workerMax);
	if (!c->threadContext->workers) {
		ft_printf_fd(2, "Error: threadWorkersInit: malloc failed\n");
		return (0);
	}
	ft_printf_fd(1, CYAN"Thread Init: %d workers\n"RESET, (s32)c->threadContext->workerMax);
	return (1);
}

/**
 * @brief Get the first free worker thread (Mtx locked when called)
 * @param c Context
 * @return The index of the first free worker thread, -1 if no worker is free
*/
s32 threadFreeWorkerGet (ThreadContext *threadContext) {
	for (s32 i = 0; i < threadContext->workerMax ; ++i) {
		if (threadContext->workers[i].busy == WORKER_FREE) {
			return (i);
		}
	}
	return (-1);
}


/* Tmp need to replace */
void lst_popfront(t_list **lst) {
	if (!*lst)
		return ;
	t_list *tmp = *lst;
	*lst = (*lst)->next;
	free(tmp);
}


/**
 * @brief Load chunks in a thread
 * @param data ThreadData
 * @return 1 (int function required by thrd_create)
*/
int threadChunksLoad(void *data) {
	ThreadData *t = (ThreadData *)data;
	Chunks *chunks = chunksLoad(t->c->perlinNoise, t->chunkX, t->chunkZ);
	mtx_lock(t->mtx);
	hashmap_set_entry(t->c->world->chunksMap, CHUNKS_MAP_ID_GET(t->chunkX, t->chunkZ), chunks);
	t->c->threadContext->workers[t->threadID].busy = WORKER_FREE;
	mtx_unlock(t->mtx);
	free(data); /* tocheck*/
	return (1);
}

/**
 * @brief Initialize a thread to load a chunk
 * @param c Context
 * @param mtx Mutex
 * @param chunkX Chunk X
 * @param chunkZ Chunk Z
 * @return 1 if success, 0 if failed to init the thread (cause thread are all busy)
*/
s8 threadInitChunkLoad(Context *c, Mutex *mtx, s32 chunkX, s32 chunkZ) {
	ThreadData	*tdata;
	s32			threadID = -1; 


	if (!(tdata = malloc(sizeof(ThreadData)))) {
		return (FALSE);
	}	

	mtx_lock(mtx);
	tdata->c = c;
	tdata->mtx = mtx;
	tdata->chunkX = chunkX;
	tdata->chunkZ = chunkZ;
	if ((threadID = threadFreeWorkerGet(c->threadContext)) == -1) {
		ft_printf_fd(1, "Thread is full, store data in lst, size: %d\n", hashmap_size(c->threadContext->chunksMapToLoad));
		// ft_lstadd_front(&c->threadContext->chunksQueue, ft_lstnew(tdata));
		mtx_unlock(mtx);
		return (FALSE);
	}
	tdata->threadID = threadID;
	ft_printf_fd(1, ORANGE"Thread: %d"RESET""CYAN" create [%d][%d], size: %u\n"RESET, threadID, chunkX, chunkZ, hashmap_size(c->threadContext->chunksMapToLoad));
	c->threadContext->workerCurrent += 1;
	c->threadContext->workers[threadID].busy = WORKER_BUSY;
	c->threadContext->workers[threadID].data = tdata;
	mtx_unlock(mtx);

	thrd_create(&c->threadContext->workers[threadID].thread, threadChunksLoad, tdata);
	return (TRUE);
}


/**
 * @brief Wait for all the worker threads to finish their job
 * @param c Context
*/
void threadWaitForWorker(Context *c) {
	s64 i = 0, max = 0;
	mtx_lock(&c->threadContext->mtx);
	max = c->threadContext->workerMax;
	// busy = c->threadContext->workerCurrent;
	mtx_unlock(&c->threadContext->mtx);
	while (i < max) {
		// if (c->threadContext->workers[i].busy == WORKER_BUSY) {
			thrd_join(c->threadContext->workers[i].thread, NULL);
			// busy--;
		// }
		++i;
	}
}


/* 
	Start by creating a supervisor thread that will manage the worker threads
	In this supervisor thread, we will create a queue of chunks to load.
	We will then create a worker thread that will load the chunks from the queue and 
	store it in chunksHashMap . When the 'mandatory chunks' are created, we can send
	a signal to main thread to start rendering the chunks.
	In background, the worker thread will continue to load chunks from the queue and the supervisor 
	continue to join them and add desired load chunks in the queue. 
	Actualy the queue is a simple linked list of ThreadData structure. Maybe need to replace it by a
	more efficient structure, hashmap or array of ThreadData. (simply to load nearest chunks first)
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

s8 chunksQueueHandling(Context *c, s32 chunkX, s32 chunkZ) {
	ThreadData *tdata;
	if (!(tdata = malloc(sizeof(ThreadData)))) {
		return (FALSE);
	}
	mtx_lock(&c->threadContext->mtx);
	/* Locked */
	tdata->c = c;
	tdata->mtx = &c->threadContext->mtx;
	tdata->chunkX = chunkX;
	tdata->chunkZ = chunkZ;
	/* If chunks not in chunksMapToload */
	if (!hashmap_get(c->threadContext->chunksMapToLoad, CHUNKS_MAP_ID_GET(chunkX, chunkZ)) && !workerIsLoadingChunks(c, chunkX, chunkZ)) {
		hashmap_set_entry(c->threadContext->chunksMapToLoad, CHUNKS_MAP_ID_GET(chunkX, chunkZ), tdata);
	}
	// ft_lstadd_front(chunksQueue, ft_lstnew(tdata));
	/* Unlocked */
	mtx_unlock(&c->threadContext->mtx);
	return (TRUE);
}


/**
 * @brief Scan the environment to load chunks arround the camera
 * @param c Context pointer
 * @param curr_x Player's x position
 * @param curr_z Player's z position
 * @param radius The radius around the player to scan
*/
s8 threadChunksLoadArround(Context *c, s32 radius) {
	s32  currentX = c->cam.chunkPos[0];
	s32  currentZ = c->cam.chunkPos[2];
	for (s32 i = -radius; i < radius; ++i) {
		for (s32 j = -radius; j < radius; ++j) {
			BlockPos pos = CHUNKS_MAP_ID_GET(currentX + i, currentZ + j);
			
			mtx_lock(&c->threadContext->mtx);
			Chunks *chunks = hashmap_get(c->world->chunksMap, pos);
			mtx_unlock(&c->threadContext->mtx);
			if (!chunks) {
				if (!chunksQueueHandling(c, pos.y, pos.z)) {
					ft_printf_fd(2, "Error: threadChunksLoadArround: chunksQueueHandling failed\n");
					return (FALSE);
				}
			}
		}
	}
	return (TRUE);
}


ThreadData *chunksToLoadNearestGet(Context *c, HashMap *chunksMapToLoad) {
	ThreadData *tdata = NULL;
	s32 distance = 0;
	s32 tmpDistance = 0;
	BlockPos pos = {0, 0, 0};
	HashMap_it it = hashmap_iterator(chunksMapToLoad);
	s8 next = 0;


	while ((next = hashmap_next(&it))) {
		pos = ((HashMap_entry *)it._current->content)->origin_data;

		tmpDistance = abs(c->cam.chunkPos[0] - pos.y) + abs(c->cam.chunkPos[2] - pos.z);
		if (distance == 0 || tmpDistance < distance) {
			distance = tmpDistance;
			tdata = (ThreadData *)it.value;
		}
	}
	return (tdata);
}

s32 threadHandling(void *context) {
	Context *c = (Context *)context;
	ThreadData *tdata = NULL;
	size_t mapSize = 0;


	if (!threadWorkersInit(c)) {
		ft_printf_fd(2, "Error: threadWorkersInit failed\n");
		return (FALSE);
	}

	while (voxIsRunning(c)) {
		threadChunksLoadArround(c, 1);
		mtx_lock(&c->threadContext->mtx);
		mapSize = hashmap_size(c->threadContext->chunksMapToLoad);
		mtx_unlock(&c->threadContext->mtx);
		if (mapSize > 0) {
			mtx_lock(&c->threadContext->mtx);
			tdata = chunksToLoadNearestGet(c, c->threadContext->chunksMapToLoad);
			mtx_unlock(&c->threadContext->mtx);
			/* Lock in this */
			if (threadInitChunkLoad(c, &c->threadContext->mtx, tdata->chunkX, tdata->chunkZ)) {
				mtx_lock(&c->threadContext->mtx);
				hashmap_remove_entry(c->threadContext->chunksMapToLoad, CHUNKS_MAP_ID_GET(tdata->chunkX, tdata->chunkZ));
				mapSize = hashmap_size(c->threadContext->chunksMapToLoad);
				// ft_printf_fd(1, RESET_LINE""PINK"Workers queue busy %u chunks remaining to load"RESET, mapSize);
				mtx_unlock(&c->threadContext->mtx);
			}
		} else {
			/* If chunks queue is empty we can wait 10 milisec before rescan arround */
			 usleep(10000);
		}
	}

	threadWaitForWorker(c);
	return (TRUE);
}


s8 threadSupervisorInit(Context *c) {
	c->threadContext = ft_calloc(sizeof(ThreadContext), 1);
	if (!c->threadContext) {
		ft_printf_fd(2, "Error: threadSupervisorInit: malloc failed\n");
		return (FALSE);
	}
	c->threadContext->chunksMapToLoad = hashmap_init(HASHMAP_SIZE_100, hashmap_entry_free);
	if (!c->threadContext->chunksMapToLoad) {
		ft_printf_fd(2, "Error: threadSupervisorInit: hashmap_init failed\n");
		return (FALSE);
	}
	mtx_init(&c->threadContext->mtx, mtx_plain);
	thrd_create(&c->threadContext->supervisor, threadHandling, c);

	return (TRUE);
}