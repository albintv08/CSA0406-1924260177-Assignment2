
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define CAMERAS 16
#define PLATES_PER_CAMERA 5000000

typedef struct {
    int camera_id;
    long plates_processed;
} CameraTask;

typedef struct {
    CameraTask tasks[CAMERAS];
    int next_task;
    pthread_mutex_t lock;
} ThreadPool;

void process_camera(CameraTask *task)
{
    volatile unsigned long result = 0;

    for (long i = 0; i < PLATES_PER_CAMERA; i++) {
        result += (unsigned long)(i % 97);
    }

    task->plates_processed = PLATES_PER_CAMERA;

    /* Prevent the compiler from removing the computation. */
    if (result == 0)
        printf("Camera %d: %lu\n",
               task->camera_id, result);
}

void *camera_worker(void *arg)
{
    CameraTask *task = (CameraTask *)arg;
    process_camera(task);
    return NULL;
}

void *pool_worker(void *arg)
{
    ThreadPool *pool = (ThreadPool *)arg;

    while (1) {
        pthread_mutex_lock(&pool->lock);

        int index = pool->next_task++;

        pthread_mutex_unlock(&pool->lock);

        if (index >= CAMERAS)
            break;

        process_camera(&pool->tasks[index]);
    }

    return NULL;
}

double elapsed_seconds(struct timespec start,
                       struct timespec end)
{
    return (end.tv_sec - start.tv_sec) +
           (end.tv_nsec - start.tv_nsec) / 1000000000.0;
}

int main(void)
{
    pthread_t threads[CAMERAS];
    CameraTask tasks[CAMERAS];

    struct timespec start, end;

    printf("TRAFFIC CAMERA THREAD BENCHMARK\n");
    printf("Cameras: %d\n", CAMERAS);
    printf("Work per camera: %d iterations\n\n",
           PLATES_PER_CAMERA);

    /* Experiment 1: One thread per camera */
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < CAMERAS; i++) {
        tasks[i].camera_id = i + 1;
        tasks[i].plates_processed = 0;

        if (pthread_create(&threads[i], NULL,
                           camera_worker, &tasks[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }

    for (int i = 0; i < CAMERAS; i++)
        pthread_join(threads[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &end);

    double one_thread_time = elapsed_seconds(start, end);

    printf("ONE THREAD PER CAMERA\n");
    printf("Threads created: %d\n", CAMERAS);
    printf("Execution time: %.6f seconds\n\n",
           one_thread_time);

    /* Experiment 2: Fixed thread pools */
    int pool_sizes[] = {4, 8, 16};

    for (int s = 0; s < 3; s++) {
        int workers = pool_sizes[s];

        ThreadPool pool;
        pool.next_task = 0;
        pthread_mutex_init(&pool.lock, NULL);

        for (int i = 0; i < CAMERAS; i++) {
            pool.tasks[i].camera_id = i + 1;
            pool.tasks[i].plates_processed = 0;
        }

        clock_gettime(CLOCK_MONOTONIC, &start);

        for (int i = 0; i < workers; i++) {
            if (pthread_create(&threads[i], NULL,
                               pool_worker, &pool) != 0) {
                perror("pthread_create");
                return 1;
            }
        }

        for (int i = 0; i < workers; i++)
            pthread_join(threads[i], NULL);

        clock_gettime(CLOCK_MONOTONIC, &end);

        double pool_time = elapsed_seconds(start, end);
        double speedup = one_thread_time / pool_time;

        printf("FIXED THREAD POOL: %d WORKERS\n", workers);
        printf("Threads created: %d\n", workers);
        printf("Cameras processed: %d\n", CAMERAS);
        printf("Execution time: %.6f seconds\n", pool_time);
        printf("Speedup: %.2fx\n\n", speedup);

        pthread_mutex_destroy(&pool.lock);
    }

    printf("Benchmark completed successfully.\n");

    return 0;
}

