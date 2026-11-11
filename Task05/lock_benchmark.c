
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <time.h>

#define THREADS 4
#define TRAINS 4
#define COACHES 8
#define BOOKINGS_PER_THREAD 200000

enum {
    GLOBAL_MUTEX,
    TRAIN_MUTEX,
    COACH_MUTEX,
    ATOMIC_SPIN
};

pthread_mutex_t global_lock;
pthread_mutex_t train_locks[TRAINS];
pthread_mutex_t coach_locks[TRAINS][COACHES];

atomic_flag spin_lock = ATOMIC_FLAG_INIT;

long bookings[TRAINS][COACHES];

typedef struct {
    int id;
    int mode;
} ThreadData;

void acquire_spinlock(void)
{
    while (atomic_flag_test_and_set_explicit(
               &spin_lock, memory_order_acquire)) {
        /* Busy waiting */
    }
}

void release_spinlock(void)
{
    atomic_flag_clear_explicit(&spin_lock, memory_order_release);
}

void *booking_worker(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    int id = data->id;
    int mode = data->mode;

    for (int i = 0; i < BOOKINGS_PER_THREAD; i++) {

        int train = (i * 7 + id * 3) % TRAINS;
        int coach = (i * 13 + id * 5) % COACHES;

        if (mode == GLOBAL_MUTEX) {
            pthread_mutex_lock(&global_lock);
            bookings[train][coach]++;
            pthread_mutex_unlock(&global_lock);
        }

        else if (mode == TRAIN_MUTEX) {
            pthread_mutex_lock(&train_locks[train]);
            bookings[train][coach]++;
            pthread_mutex_unlock(&train_locks[train]);
        }

        else if (mode == COACH_MUTEX) {
            pthread_mutex_lock(&coach_locks[train][coach]);
            bookings[train][coach]++;
            pthread_mutex_unlock(&coach_locks[train][coach]);
        }

        else if (mode == ATOMIC_SPIN) {
            acquire_spinlock();
            bookings[train][coach]++;
            release_spinlock();
        }
    }

    return NULL;
}

void run_benchmark(int mode, const char *name)
{
    pthread_t threads[THREADS];
    ThreadData data[THREADS];

    long total = 0;

    for (int i = 0; i < TRAINS; i++) {
        for (int j = 0; j < COACHES; j++) {
            bookings[i][j] = 0;
        }
    }

    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < THREADS; i++) {
        data[i].id = i;
        data[i].mode = mode;

        pthread_create(&threads[i], NULL,
                       booking_worker, &data[i]);
    }

    for (int i = 0; i < THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    for (int i = 0; i < TRAINS; i++) {
        for (int j = 0; j < COACHES; j++) {
            total += bookings[i][j];
        }
    }

    double elapsed = (end.tv_sec - start.tv_sec) +
                     (end.tv_nsec - start.tv_nsec) / 1e9;

    double throughput = total / elapsed;

    printf("\nLocking method: %s\n", name);
    printf("Threads: %d\n", THREADS);
    printf("Total bookings: %ld\n", total);
    printf("Expected bookings: %d\n",
           THREADS * BOOKINGS_PER_THREAD);
    printf("Execution time: %.6f seconds\n", elapsed);
    printf("Throughput: %.0f bookings/second\n", throughput);
    printf("Verification: %s\n",
           total == THREADS * BOOKINGS_PER_THREAD
               ? "SUCCESS" : "FAILED");
}

int main(void)
{
    pthread_mutex_init(&global_lock, NULL);

    for (int i = 0; i < TRAINS; i++) {
        pthread_mutex_init(&train_locks[i], NULL);

        for (int j = 0; j < COACHES; j++) {
            pthread_mutex_init(&coach_locks[i][j], NULL);
        }
    }

    atomic_flag_clear(&spin_lock);

    printf("RAILWAY BOOKING LOCK GRANULARITY BENCHMARK\n");
    printf("Threads: %d | Bookings per thread: %d\n",
           THREADS, BOOKINGS_PER_THREAD);

    run_benchmark(GLOBAL_MUTEX, "Global Mutex");
    run_benchmark(TRAIN_MUTEX, "Per-Train Mutex");
    run_benchmark(COACH_MUTEX, "Per-Coach Mutex");
    run_benchmark(ATOMIC_SPIN, "Atomic Spinlock");

    pthread_mutex_destroy(&global_lock);

    for (int i = 0; i < TRAINS; i++) {
        pthread_mutex_destroy(&train_locks[i]);

        for (int j = 0; j < COACHES; j++) {
            pthread_mutex_destroy(&coach_locks[i][j]);
        }
    }

    return 0;
}
