#ifndef HEADER_THREAD_LOAD_H
#define HEADER_THREAD_LOAD_H

#include <stdio.h>
#include "../src/tinycthread/tinycthread.h"        /* TinyCThread functions */
#include <unistd.h>
#include "world.h"

typedef thrd_t Thread;
typedef mtx_t Mutex;

typedef struct s_thread_data {
	Context *c;
	Mutex	*mtx;
	s32 	chunkX;
	s32 	chunkZ;
} ThreadData;

#define WORKER_FREE 0
#define WORKER_BUSY 1

typedef struct s_thread_entity {
	Thread		thread;
	s8			busy;
	ThreadData	*data;
} ThreadEntity;

struct s_worker_thread {
    ThreadEntity	*workers;
    s64         	max;
    s64         	current;
	t_list 			*dataQueue;
};

/**
 * @brief Get the number of threads available on the system
 * @return The number of threads available on the system
*/
FT_INLINE s64 ThreadsAvailableGet() {
	s64 num_threads = sysconf(_SC_NPROCESSORS_ONLN);
    if (num_threads == -1) {
        perror("sysconf");
        return 1;
    }
	return (num_threads);
}

s8 threadInit(Context *c);
void threadWaitForWorker(Context *c);
s8 theadInitChunkLoad(Context *c, Mutex *mtx, s32 chunkX, s32 chunkZ);

#endif /* HEADER_THREAD_LOAD_H */
