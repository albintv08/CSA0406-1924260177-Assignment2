#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define BUFFER_SIZE 5
#define ITEMS_PER_PRODUCER 20
#define MAX_THREADS 4

int buffer[BUFFER_SIZE];
int in = 0, out = 0;

sem_t empty_slots, full_slots;

int next_item = 1;
int consumed_count = 0;
long consumed_sum = 0;
int consumed_items[MAX_THREADS * ITEMS_PER_PRODUCER];

int producer_count = 1;
int consumer_count = 1;

void *producer(void *arg)
{
    int id = *(int *)arg;

    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        sem_wait(&empty_slots);

        int item = next_item++;
        buffer[in] = item;
        in = (in + 1) % BUFFER_SIZE;

        printf("Producer %d produced item %d\n", id, item);

        sem_post(&full_slots);
    }

    return NULL;
}

void *consumer(void *arg)
{
    int id = *(int *)arg;

    for (int i = 0; i < (producer_count * ITEMS_PER_PRODUCER)
                            / consumer_count; i++) {
        sem_wait(&full_slots);

        int item = buffer[out];
        out = (out + 1) % BUFFER_SIZE;

        consumed_items[consumed_count++] = item;
consumed_sum += item;

        printf("Consumer %d consumed item %d\n", id, item);

        sem_post(&empty_slots);
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc == 3) {
        producer_count = atoi(argv[1]);
        consumer_count = atoi(argv[2]);
    }

    if (producer_count < 1 || producer_count > MAX_THREADS ||
        consumer_count < 1 || consumer_count > MAX_THREADS ||
        (producer_count * ITEMS_PER_PRODUCER) % consumer_count != 0) {
        printf("Usage: %s [producers 1-4] [consumers 1-4]\n", argv[0]);
        return 1;
    }

    pthread_t producers[MAX_THREADS], consumers[MAX_THREADS];
    int producer_ids[MAX_THREADS], consumer_ids[MAX_THREADS];

    sem_init(&empty_slots, 0, BUFFER_SIZE);
    sem_init(&full_slots, 0, 0);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < producer_count; i++) {
        producer_ids[i] = i + 1;
        pthread_create(&producers[i], NULL, producer, &producer_ids[i]);
    }

    for (int i = 0; i < consumer_count; i++) {
        consumer_ids[i] = i + 1;
        pthread_create(&consumers[i], NULL, consumer, &consumer_ids[i]);
    }

    for (int i = 0; i < producer_count; i++)
        pthread_join(producers[i], NULL);

    for (int i = 0; i < consumer_count; i++)
        pthread_join(consumers[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed = (end.tv_sec - start.tv_sec) +
                     (end.tv_nsec - start.tv_nsec) / 1e9;

    int total = producer_count * ITEMS_PER_PRODUCER;
    int valid = consumed_count == total;

    for (int i = 1; i <= total; i++) {
        int occurrences = 0;
        for (int j = 0; j < consumed_count; j++) {
            if (consumed_items[j] == i)
                occurrences++;
        }
        if (occurrences != 1)
            valid = 0;
    }

    printf("\n=== SYNCHRONIZATION RESULTS ===\n");
    printf("Producers: %d\n", producer_count);
    printf("Consumers: %d\n", consumer_count);
    printf("Items produced: %d\n", total);
    printf("Items consumed: %d\n", consumed_count);
printf("Consumed item sum: %ld\n", consumed_sum);
    printf("Exactly-once verification: %s\n",
           valid ? "SUCCESS" : "FAILED");
    printf("Execution time: %.6f seconds\n", elapsed);

    sem_destroy(&empty_slots);
    sem_destroy(&full_slots);

    return valid ? 0 : 1;
}
