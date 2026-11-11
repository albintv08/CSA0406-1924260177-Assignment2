
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define P 5
#define R 3

int available[R] = {3, 3, 2};

int maximum[P][R] = {
    {7, 5, 3},
    {3, 2, 2},
    {9, 0, 2},
    {2, 2, 2},
    {4, 3, 3}
};

int allocation[P][R] = {
    {0, 1, 0},
    {2, 0, 0},
    {3, 0, 2},
    {2, 1, 1},
    {0, 0, 2}
};

int need[P][R];

void calculate_need(void)
{
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < R; j++) {
            need[i][j] = maximum[i][j] - allocation[i][j];
        }
    }
}

int safety_algorithm(int sequence[P])
{
    int work[R];
    int finish[P] = {0};
    int count = 0;

    for (int j = 0; j < R; j++)
        work[j] = available[j];

    while (count < P) {
        int found = 0;

        for (int i = 0; i < P; i++) {
            if (finish[i])
                continue;

            int can_finish = 1;

            for (int j = 0; j < R; j++) {
                if (need[i][j] > work[j]) {
                    can_finish = 0;
                    break;
                }
            }

            if (can_finish) {
                for (int j = 0; j < R; j++)
                    work[j] += allocation[i][j];

                sequence[count++] = i;
                finish[i] = 1;
                found = 1;
            }
        }

        if (!found)
            return 0;
    }

    return 1;
}

void print_sequence(const int sequence[P])
{
    for (int i = 0; i < P; i++) {
        printf("P%d", sequence[i]);
        if (i < P - 1)
            printf(" -> ");
    }
    printf("\n");
}

int request_resources(int process, int request[R])
{
    for (int j = 0; j < R; j++) {
        if (request[j] > need[process][j]) {
            printf("Request exceeds declared maximum.\n");
            return 0;
        }
        if (request[j] > available[j]) {
            printf("Request cannot be granted: resources unavailable.\n");
            return 0;
        }
    }

    /* Temporarily allocate the requested resources. */
    for (int j = 0; j < R; j++) {
        available[j] -= request[j];
        allocation[process][j] += request[j];
        need[process][j] -= request[j];
    }

    int sequence[P];

    if (safety_algorithm(sequence)) {
        printf("Request granted. Safe sequence: ");
        print_sequence(sequence);
        return 1;
    }

    /* Roll back an unsafe tentative allocation. */
    for (int j = 0; j < R; j++) {
        available[j] += request[j];
        allocation[process][j] -= request[j];
        need[process][j] += request[j];
    }

    printf("Request denied: allocation would create an unsafe state.\n");
    return 0;
}

void reset_state(void)
{
    int initial_available[R] = {3, 3, 2};

    int initial_allocation[P][R] = {
        {0, 1, 0},
        {2, 0, 0},
        {3, 0, 2},
        {2, 1, 1},
        {0, 0, 2}
    };

    memcpy(available, initial_available, sizeof(available));
    memcpy(allocation, initial_allocation, sizeof(allocation));
    calculate_need();
}

void performance_test(int process_count)
{
    int (*test_max)[R] = calloc(process_count, sizeof(*test_max));
    int (*test_alloc)[R] = calloc(process_count, sizeof(*test_alloc));
    int (*test_need)[R] = calloc(process_count, sizeof(*test_need));
    int *finish = calloc(process_count, sizeof(*finish));
    int work[R] = {3, 3, 2};

    if (!test_max || !test_alloc || !test_need || !finish) {
        printf("Memory allocation failed for %d processes.\n",
               process_count);
        free(test_max);
        free(test_alloc);
        free(test_need);
        free(finish);
        return;
    }

    /* Generate a safe workload with progressively eligible processes. */
for (int i = 0; i < process_count; i++) {
    test_alloc[i][0] = 1;
    test_alloc[i][1] = 0;
    test_alloc[i][2] = 0;

    test_need[i][0] = 3 + (process_count - 1 - i);
    test_need[i][1] = 1;
    test_need[i][2] = 1;

    for (int j = 0; j < R; j++) {
        test_max[i][j] = test_alloc[i][j] + test_need[i][j];
    }
}

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    int completed = 0;

    while (completed < process_count) {
        int found = 0;

        for (int i = 0; i < process_count; i++) {
            if (finish[i])
                continue;

            int can_finish = 1;

            for (int j = 0; j < R; j++) {
                if (test_need[i][j] > work[j]) {
                    can_finish = 0;
                    break;
                }
            }

            if (can_finish) {
                for (int j = 0; j < R; j++)
                    work[j] += test_alloc[i][j];

                finish[i] = 1;
                completed++;
                found = 1;
            }
        }

        if (!found)
            break;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed = (end.tv_sec - start.tv_sec) +
                     (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Processes: %d | Safe: %s | Completed: %d | Time: %.9f seconds\n",
           process_count,
           completed == process_count ? "YES" : "NO",
           completed, elapsed);

    free(test_max);
    free(test_alloc);
    free(test_need);
    free(finish);
}

int main(void)
{
    int sequence[P];

    printf("BANKER'S ALGORITHM - DEADLOCK AVOIDANCE\n");

    reset_state();

    printf("\nInitial system state:\n");
    if (safety_algorithm(sequence)) {
        printf("System is SAFE.\nSafe sequence: ");
        print_sequence(sequence);
    } else {
        printf("System is UNSAFE.\n");
    }

    printf("\nRequest 1: P1 requests (1, 0, 2)\n");
    request_resources(1, (int[]){1, 0, 2});

    reset_state();

    printf("\nRequest 2: P4 requests (3, 3, 0)\n");
    request_resources(4, (int[]){3, 3, 0});

    reset_state();

    printf("\nRequest 3: P0 requests (0, 2, 0)\n");
    request_resources(0, (int[]){0, 2, 0});

    printf("\nPERFORMANCE TEST\n");
    performance_test(10);
    performance_test(100);
    performance_test(1000);

    return 0;
}
