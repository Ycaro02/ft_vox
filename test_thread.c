#include <stdio.h>
#include <threads.h>
#include <unistd.h>

mtx_t mutex;

typedef struct {
	int		data;
	mtx_t	*mutex;
} ThreadData;

int update_shared_data(void *shared_data) {
    mtx_lock(&mutex);
	int *data = (int *)shared_data;
    (*data)++;
    printf("Thread %ld updated shared data to: %d\n", thrd_current(), (*data));
    mtx_unlock(&mutex);
	return (0);
}



int main() {
	int shared_data = 0;
    mtx_init(&mutex, mtx_plain);

    thrd_t threads[5];
    for (int i = 0; i < 5; i++) {
        thrd_create(&threads[i], update_shared_data, &shared_data);
    }

    for (int i = 0; i < 5; i++) {
        thrd_join(threads[i], NULL);
    }

    mtx_destroy(&mutex);
    return 0;
}
