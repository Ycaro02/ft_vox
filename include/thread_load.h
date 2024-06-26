#ifndef HEADER_THREAD_LOAD_H
#define HEADER_THREAD_LOAD_H

#include "typedef_struct.h"							/* Struct typedef */

#define LOAD_PRIORITY_LOW 		0U
#define LOAD_PRIORITY_HIGH 		1U

/* Thread data */
struct s_thread_data {
	Context *c;
	Mutex	*chunkMtx;
	s32 	chunkX;
	s32 	chunkZ;
	s8		threadID;
	u8 		priority;
};

/* Workers status */
#define WORKER_FREE 0
#define WORKER_BUSY 1

/* Thread entity */
struct s_thread_entity {
	Thread		thread;
	s8			busy;
	ThreadData	*data;
};

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
