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

/**
 * @brief Load chunks in a thread
 * @param data ThreadData
 * @return 1 (int function required by thrd_create)
*/
int threadChunksLoad(void *data) {
	ThreadData *t = (ThreadData *)data;
	Chunks *chunks = chunksLoad(t->mtx, t->c->perlin2D, t->chunkX, t->chunkZ);
	mtx_lock(t->mtx);
	hashmap_set_entry(t->c->world->chunksMap, CHUNKS_MAP_ID_GET(t->chunkX, t->chunkZ), chunks);
	t->c->threadContext->workers[t->threadID].busy = WORKER_FREE;
	mtx_unlock(t->mtx);
	free(data); /* tocheck*/
	return (1);
}


// s32 chunkDistanceGet(s32 camChunkX, s32 camChunkZ, s32 chunkX, s32 chunkZ);

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


	mtx_lock(mtx);
	if ((threadID = threadFreeWorkerGet(c->threadContext)) == -1) {
		// ft_printf_fd(1, RED"Thread is full\n"RESET, hashmap_size(c->threadContext->chunksMapToLoad));
		mtx_unlock(mtx);
		return (FALSE);
	} else if (!(tdata = malloc(sizeof(ThreadData)))) {
		ft_printf_fd(2, "Error: threadInitChunkLoad: malloc failed\n");
		mtx_unlock(mtx);
		return (FALSE);
	}
	tdata->c = c;
	tdata->mtx = mtx;
	tdata->chunkX = chunkX;
	tdata->chunkZ = chunkZ;
	tdata->threadID = threadID;
	// ft_printf_fd(1, ORANGE"\nThread: %d"RESET""CYAN" create [%d][%d], "RESET""PINK"CamChunksPos: [%d][%d] -> Distance: |%d|"RESET, threadID, chunkX, chunkZ, c->cam.chunkPos[0], c->cam.chunkPos[2], chunkDistanceGet(c->cam.chunkPos[0], c->cam.chunkPos[2], chunkX, chunkZ));
	c->threadContext->workerCurrent += 1;
	c->threadContext->workers[threadID].busy = WORKER_BUSY;
	c->threadContext->workers[threadID].data = tdata;
	mtx_unlock(mtx);

	thrd_create(&c->threadContext->workers[threadID].thread, threadChunksLoad, tdata);
	if (!thrd_detach(c->threadContext->workers[threadID].thread)) {
		ft_printf_fd(2, "Error: threadInitChunkLoad: thrd_detach failed\n");
		return (FALSE);
	}
	return (TRUE);
}




/**
 * @brief Wait for all the worker threads to finish their job
 * @param c Context
*/
void supervisorWaitWorker(Context *c) {
	s64 i = 0, max = 0;
	s32 status = 0;
	mtx_lock(&c->threadContext->mtx);
	max = c->threadContext->workerMax;
	mtx_unlock(&c->threadContext->mtx);

	while (i < max) {
		mtx_lock(&c->threadContext->mtx);
		status = c->threadContext->workers[i].busy;
		while (status == WORKER_BUSY) {
			mtx_unlock(&c->threadContext->mtx);
			usleep(10000);
			mtx_lock(&c->threadContext->mtx);
			status = c->threadContext->workers[i].busy;
		}
		mtx_unlock(&c->threadContext->mtx);
		ft_printf_fd(1, ORANGE"\nWorkers Thread: "RESET""YELLOW"[%d]"RESET""GREEN" finished: status %d"RESET, i, status);
		++i;
	}
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

/**
 * @brief Lock before to call this function
 * @brief Add chunks to the queue to load if not already in the queue
 * @param c Context
 * @param chunkX Chunk X
 * @param chunkZ Chunk Z
 * @return TRUE if success, FALSE if failed to add the chunks to the queue
*/
s8 chunksQueueHandling(Context *c, s32 chunkX, s32 chunkZ) {
	ThreadData *tdata = NULL;

	if (hashmap_get(c->threadContext->chunksMapToLoad, CHUNKS_MAP_ID_GET(chunkX, chunkZ))\
		|| workerIsLoadingChunks(c, chunkX, chunkZ)) {
		return (TRUE);
	} else if (!(tdata = ft_calloc(sizeof(ThreadData), 1))) {
		return (FALSE);
	}

	tdata->c = c;
	tdata->mtx = &c->threadContext->mtx;
	tdata->chunkX = chunkX;
	tdata->chunkZ = chunkZ;
	/* If chunks not in chunksMapToload */
	hashmap_set_entry(c->threadContext->chunksMapToLoad, CHUNKS_MAP_ID_GET(chunkX, chunkZ), tdata);
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
	mtx_lock(&c->threadContext->mtx);
	s32  currentX = c->cam.chunkPos[0];
	s32  currentZ = c->cam.chunkPos[2];
	mtx_unlock(&c->threadContext->mtx);
	for (s32 i = -radius; i <= radius; ++i) {
		for (s32 j = -radius; j <= radius; ++j) {
			BlockPos pos = CHUNKS_MAP_ID_GET(currentX + i, currentZ + j);
			
			mtx_lock(&c->threadContext->mtx);
			Chunks *chunks = hashmap_get(c->world->chunksMap, pos);
			if (!chunks) {
				if (!chunksQueueHandling(c, pos.y, pos.z)) {
					ft_printf_fd(2, "Error: threadChunksLoadArround: chunksQueueHandling failed\n");
					mtx_unlock(&c->threadContext->mtx);
					return (FALSE);
				}
			}
			mtx_unlock(&c->threadContext->mtx);
		}
	}
	return (TRUE);
}

s32 chunkDistanceGet(s32 camChunkX, s32 camChunkZ, s32 chunkX, s32 chunkZ) {
	return (abs(camChunkX - chunkX) + abs(camChunkZ - chunkZ));
}

/**
 * @brief Get the nearest chunks to load from the queue
 * @param c Context
 * @param chunksMapToLoad HashMap of chunks to load
 * @return ThreadData pointer to the nearest chunks to load
*/
ThreadData *chunksToLoadNearestGet(Context *c, HashMap *chunksMapToLoad) {
	BlockPos 	pos = {0, 0, 0};
	HashMap_it	it = hashmap_iterator(chunksMapToLoad);
	ThreadData	*tdata = NULL;
	s32			distance = -1, tmpDistance = 0;
	s8 			next = 0;

	while ((next = hashmap_next(&it))) {
		pos = ((HashMap_entry *)it._current->content)->origin_data;
		// tmpDistance = abs(c->cam.chunkPos[0] - pos.y) + abs(c->cam.chunkPos[2] - pos.z);
		tmpDistance = chunkDistanceGet(c->cam.chunkPos[0], c->cam.chunkPos[2], pos.y, pos.z);
		if (distance == -1 || tmpDistance <= distance) {
			distance = tmpDistance;
			tdata = (ThreadData *)it.value;
		}
	}
	return (tdata);
}

/**
 * @brief Thread supervisor function, manage the worker threads
 * @param context Context
 * @return TRUE if success, FALSE if failed
*/
s32 threadHandling(void *context) {
	Context *c = (Context *)context;
	ThreadData *tdata = NULL;
	size_t mapSize = 0;


	if (!threadWorkersInit(c)) {
		ft_printf_fd(2, "Error: threadWorkersInit failed\n");
		return (FALSE);
	}

	while (voxIsRunning(c)) {
		threadChunksLoadArround(c, CHUNKS_LOAD_RADIUS);
		mtx_lock(&c->threadContext->mtx);
		mapSize = hashmap_size(c->threadContext->chunksMapToLoad);
		mtx_unlock(&c->threadContext->mtx);
		if (mapSize > 0) {
			mtx_lock(&c->threadContext->mtx);
			tdata = chunksToLoadNearestGet(c, c->threadContext->chunksMapToLoad);
			mtx_unlock(&c->threadContext->mtx);
			/* Lock in this */
			if (tdata && threadInitChunkLoad(c, &c->threadContext->mtx, tdata->chunkX, tdata->chunkZ)) {
				mtx_lock(&c->threadContext->mtx);
				hashmap_remove_entry(c->threadContext->chunksMapToLoad, CHUNKS_MAP_ID_GET(tdata->chunkX, tdata->chunkZ), HASHMAP_FREE_DATA);
				mapSize = hashmap_size(c->threadContext->chunksMapToLoad);
				// free(tdata);
				mtx_unlock(&c->threadContext->mtx);
			}
		} else {
			/* If chunks queue is empty we can wait 10 milisec before rescan arround */
			 usleep(10000);
		}
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
	c->threadContext->chunksMapToLoad = hashmap_init(HASHMAP_SIZE_100, hashmap_entry_free);
	if (!c->threadContext->chunksMapToLoad) {
		ft_printf_fd(2, "Error: threadSupervisorInit: hashmap_init failed\n");
		return (FALSE);
	}
	mtx_init(&c->threadContext->mtx, mtx_plain);
	thrd_create(&c->threadContext->supervisor, threadHandling, c);

	return (TRUE);
}

/* 
	Start by creating a supervisor thread that will manage the worker threads
	In this supervisor thread, we will create a queue of chunks to load (HashMap) .
	We will then create a worker thread that will load the chunks from the queue and 
	store it in chunksHashMap . When the 'mandatory chunks' are created, we can send
	a signal to main thread to start rendering the chunks.
	In background, the worker thread will continue to load chunks from the queue and the supervisor 
	continue to handle them. 
*/