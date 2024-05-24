#ifndef HEADER_THREAD_LOAD_H
#define HEADER_THREAD_LOAD_H

#include <stdio.h>									/* perror */
#include <unistd.h>									/* sysconf */	
#include "../src/tinycthread/tinycthread.h"			/* TinyCThread functions */
#include "world.h"									/* Context */

typedef thrd_t Thread;
typedef mtx_t Mutex;

/* Thread data */
typedef struct s_thread_data {
	Context *c;
	Mutex	*mtx;
	s32 	chunkX;
	s32 	chunkZ;
	s8		threadID;
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
 * @return The number of threads available on the system
*/
FT_INLINE s64 ThreadsAvailableGet() {
	s64 num_threads = sysconf(_SC_NPROCESSORS_ONLN);
    if (num_threads == -1) {
        perror("sysconf");
        return (1);
    }
	if (num_threads > 1) {
		num_threads -= 1;
	}
	return (num_threads);
}

/* Supervisor now init workers */
s8 threadSupervisorInit(Context *c);

#endif /* HEADER_THREAD_LOAD_H */
