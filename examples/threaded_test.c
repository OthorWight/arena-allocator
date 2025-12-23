#include <stdio.h>
#include <pthread.h>
#include <unistd.h> /* for usleep to simulate work */

/* Define Implementation in this file */
#define ARENA_IMPLEMENTATION
#include "arena.h"

#define NUM_THREADS 4
#define ITEMS_PER_THREAD 50000

/* Mutex strictly for clean printing to console (not needed for memory) */
pthread_mutex_t print_lock;

typedef struct {
    int id;
    char junk_data[64];
} TaskData;

typedef struct {
    int thread_id;
} ThreadArgs;

/* --- WORKER FUNCTION --- */
void *worker_entry(void *arg) {
    ThreadArgs *args = (ThreadArgs*)arg;
    int tid = args->thread_id;

    /* 1. Initialize Thread-Local Arena
       No locks needed here! This arena belongs 100% to this thread. */
    Arena local_arena = {0};
    arena_init(&local_arena);

    /* 2. Simulate Heavy Workload */
    /* Imagine parsing a massive JSON file or processing game entities */
    for (int i = 0; i < ITEMS_PER_THREAD; i++) {
        
        /* Allocation is blazing fast because there is no mutex contention */
        TaskData *t = arena_alloc_struct(&local_arena, TaskData);
        t->id = i;
        
        /* Occasionally use scratchpad pattern inside the thread */
        if (i % 1000 == 0) {
            ArenaTemp scratch = arena_temp_begin(&local_arena);
            char *temp_str = arena_alloc_array(&local_arena, char, 256);
            sprintf(temp_str, "Thread %d processing item %d", tid, i);
            arena_temp_end(scratch);
        }
    }

    /* 3. Report Stats */
    pthread_mutex_lock(&print_lock);
    printf("[Thread %d] Done.\n", tid);
    arena_print_stats(&local_arena); /* Prints specific stats for this thread */
    printf("\n");
    pthread_mutex_unlock(&print_lock);

    /* 4. Cleanup */
    arena_free(&local_arena);
    
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    ThreadArgs args[NUM_THREADS];
    
    pthread_mutex_init(&print_lock, NULL);

    printf("=== Multi-Threaded Arena Test ===\n");
    printf("Spawning %d threads. Each creating its own Arena.\n\n", NUM_THREADS);

    /* Spawn Workers */
    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].thread_id = i;
        pthread_create(&threads[i], NULL, worker_entry, &args[i]);
    }

    /* Wait for Workers */
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("=== All threads finished successfully ===\n");
    pthread_mutex_destroy(&print_lock);
    return 0;
}
