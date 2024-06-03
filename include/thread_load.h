#ifndef HEADER_THREAD_LOAD_H
#define HEADER_THREAD_LOAD_H

#include <stdio.h>									/* perror */
#include <unistd.h>									/* sysconf */	
#include "../src/tinycthread/tinycthread.h"			/* TinyCThread functions */
#include "world.h"									/* Context */

typedef thrd_t Thread;
typedef mtx_t Mutex;


#define LOAD_PRIORITY_LOW 		0U
#define LOAD_PRIORITY_HIGH 		1U

/* Thread data */
typedef struct s_thread_data {
	Context *c;
	Mutex	*chunkMtx;
	s32 	chunkX;
	s32 	chunkZ;
	s8		threadID;
	u8 		priority;
} ThreadData;

/* Workers status */
#define WORKER_FREE 0
#define WORKER_BUSY 1

/* Thread entity */
typedef struct s_thread_entity {
	Thread		thread;
	s8			busy;
	ThreadData	*data;
} ThreadEntity;

/**
 * @brief Get the number of threads available on the system
 * @return The number of threads available on the system minus 4 if possible else return one
*/
FT_INLINE s64 ThreadsAvailableGet() {
	s64 num_threads = sysconf(_SC_NPROCESSORS_ONLN);
	s64 decrease_thread = 4;
    if (num_threads == -1) {
        perror("sysconf");
        return (1);
    }
	while (decrease_thread > 0 && num_threads > 1) {
		num_threads -= 1;
		decrease_thread -= 1;
	}
	return (num_threads);
}

#define VOX_PROTECTED_LOG(c, msg, ...) \
    do { \
        mtx_lock(&(c->threadContext->logMtx)); \
        ft_printf_fd(1, msg, ##__VA_ARGS__); \
        mtx_unlock(&(c->threadContext->logMtx)); \
    } while (0)

/* render/thread_supervisor.c */
s8			threadSupervisorInit(Context *c);
void		chunksToLoadPrioritySet(Context *c, BlockPos chunkID, u8 priority);
ThreadData	*chunksToLoadNearestGet(Context *c, HashMap *chunksMapToLoad);
int			threadChunksLoad(void *data);


/* render/thread_workers.c */
s8 		workerIsLoadingChunks(Context *c, s32 chunkX, s32 chunkZ);
s8 		threadWorkersInit(Context *c);
void	chunksMapPopData(Context *c, ThreadData *tdata);

#endif /* HEADER_THREAD_LOAD_H */
