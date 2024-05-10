#include "../../include/thread_load.h"
#include "../../include/chunks.h"

void threadInit(Context *c) {
	mtx_init(&c->mtx, mtx_plain);
	c->thread = malloc(sizeof(WorkerThread) * 1);
	c->thread->max = ThreadsAvailableGet();
	c->thread->current = 0;
	c->thread->workers = ft_calloc(sizeof(ThreadEntity), c->thread->max);

}

s32 threadFreeWorkerGet (Context *c) {
	for (s32 i = 0; i < c->thread->max; ++i) {
		if (c->thread->workers[i].busy == WORKER_FREE) {
			return (i);
		}
	}
	return (-1);
}


void threadWaitForWorker(Context *c) {
	s32 i = 0;
	while (i < c->thread->max) {
		if (c->thread->workers[i].busy == WORKER_BUSY) {
			thrd_join(c->thread->workers[i].thread, NULL);
		}
	}
}

int threadChunksLoad(void *data) {
	ThreadData *t = (ThreadData *)data;

	Chunks *chunks = chunksLoad(t->c->perlinNoise, t->chunkX, t->chunkZ);
	mtx_lock(t->mtx);
	hashmap_set_entry(t->c->world->chunksMap, CHUNKS_MAP_ID_GET(t->chunkX, t->chunkZ), chunks);
	mtx_unlock(t->mtx);
	return (1);
}

s8 theadInitChunkLoad(Context *c, Mutex *mtx, s32 chunkX, s32 chunkZ) {
	ThreadData	*tdata;
	s32			threadID = -1; 

	if ((threadID = threadFreeWorkerGet(c)) == -1) {
		mtx_unlock(mtx);
		return (FALSE);
	}
	c->thread->current += 1;

	if (!(tdata = malloc(sizeof(ThreadData)))) {
		return (FALSE);
	}	
	tdata->c = c;
	tdata->chunkX = chunkX;
	tdata->chunkZ = chunkZ;
	tdata->mtx = mtx;
	thrd_create(&c->thread->workers[threadID].thread, threadChunksLoad, tdata);
	return (TRUE);
}