#include "../../include/thread_load.h"
#include "../../include/chunks.h"

s8 threadInit(Context *c) {
	mtx_init(&c->mtx, mtx_plain);
	c->thread = ft_calloc(sizeof(WorkerThread), 1);
	if (!c->thread) {
		ft_printf_fd(2, "Error: threadInit: malloc failed\n");
		return (0);
	}
	c->thread->max = ThreadsAvailableGet();
	c->thread->current = 0;
	c->thread->workers = ft_calloc(sizeof(ThreadEntity), c->thread->max);
	if (!c->thread->workers) {
		ft_printf_fd(2, "Error: threadInit: malloc failed\n");
		return (0);
	}
	ft_printf_fd(1, CYAN"Thread Init: %d workers\n"RESET, (s32)c->thread->max);
	return (1);
}

s32 threadFreeWorkerGet (Context *c) {
	for (s32 i = 0; i < c->thread->max; ++i) {
		if (c->thread->workers[i].busy == WORKER_FREE) {
			return (i);
		}
	}
	return (-1);
}


void lst_popfront(t_list **lst) {
	if (!*lst)
		return ;
	t_list *tmp = *lst;
	*lst = (*lst)->next;
	free(tmp);
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


	if (!(tdata = malloc(sizeof(ThreadData)))) {
		return (FALSE);
	}	

	mtx_lock(mtx);
	tdata->c = c;
	tdata->mtx = mtx;
	tdata->chunkX = chunkX;
	tdata->chunkZ = chunkZ;
	if ((threadID = threadFreeWorkerGet(c)) == -1) {
		// ft_printf_fd(1, "Thread is full, store data in lst, size: %d\n", ft_lstsize(c->thread->dataQueue));
		ft_lstadd_front(&c->thread->dataQueue, ft_lstnew(tdata));
		mtx_unlock(mtx);
		return (FALSE);
	}
	// ft_printf_fd(1, RESET_LINE""PINK"Thread: %d"RESET""CYAN" create [%d][%d]"RESET, threadID, chunkX, chunkZ);
	c->thread->current += 1;
	c->thread->workers[threadID].busy = WORKER_BUSY;
	c->thread->workers[threadID].data = tdata;
	mtx_unlock(mtx);

	thrd_create(&c->thread->workers[threadID].thread, threadChunksLoad, tdata);
	return (TRUE);
}

void threadWaitForWorker(Context *c) {
	s64 i = 0;
	while (c->thread->current > 0) {
		if (c->thread->workers[i].busy == WORKER_BUSY) {
			thrd_join(c->thread->workers[i].thread, NULL);
			if (c->thread->dataQueue != NULL) {
				ThreadData *tdata = (ThreadData *)c->thread->dataQueue->content;
				// ft_printf_fd(1, GREEN"ThreadID: %d create [%d][%d]\n"RESET, i, tdata->chunkX, tdata->chunkZ);
				c->thread->workers[i].data = tdata;
				lst_popfront(&c->thread->dataQueue);
				thrd_create(&c->thread->workers[i].thread, threadChunksLoad, tdata);
				ft_printf_fd(1, RESET_LINE""ORANGE"Workers queue busy %d chunks remaining to load"RESET, ft_lstsize(c->thread->dataQueue) );
			} else {
				c->thread->workers[i].busy = WORKER_FREE;
				c->thread->current -= 1;
			}
		}
		++i;
		if (i == c->thread->max) {
			i = 0;
		}
	}
}