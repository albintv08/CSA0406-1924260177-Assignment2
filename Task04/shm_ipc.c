#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <time.h>

#define CHUNK_SIZE (4 * 1024 * 1024)

typedef struct {
    sem_t empty;
    sem_t full;
    size_t chunk_size;
    size_t received;
    int valid;
} SharedData;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <size_in_MB>\n", argv[0]);
        return 1;
    }

    long mb = atol(argv[1]);
    if (mb <= 0) {
        fprintf(stderr, "Invalid size\n");
        return 1;
    }

    size_t total = (size_t)mb * 1024 * 1024;
    size_t map_size = sizeof(SharedData) + CHUNK_SIZE;

    const char *name = "/hospital_ipc_shm";
    int fd = shm_open(name, O_CREAT | O_RDWR, 0600);

    if (fd == -1 || ftruncate(fd, (off_t)map_size) == -1) {
        perror("shared memory");
        if (fd != -1) close(fd);
        shm_unlink(name);
        return 1;
    }

    void *map = mmap(NULL, map_size, PROT_READ | PROT_WRITE,
                     MAP_SHARED, fd, 0);
    close(fd);

    if (map == MAP_FAILED) {
        perror("mmap");
        shm_unlink(name);
        return 1;
    }

    SharedData *s = (SharedData *)map;
    char *data = (char *)map + sizeof(SharedData);

    if (sem_init(&s->empty, 1, 1) == -1 ||
        sem_init(&s->full, 1, 0) == -1) {
        perror("sem_init");
        munmap(map, map_size);
        shm_unlink(name);
        return 1;
    }

    s->received = 0;
    s->valid = 1;

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        while (1) {
            sem_wait(&s->full);

            if (s->chunk_size == 0)
                break;

            for (size_t i = 0; i < s->chunk_size; i++) {
                if (data[i] != 'A') {
                    s->valid = 0;
                    break;
                }
            }

            s->received += s->chunk_size;
            sem_post(&s->empty);
        }

        printf("Child received: %zu bytes\n", s->received);
        printf("Data validation: %s\n",
               s->valid ? "SUCCESS" : "FAILURE");
        fflush(stdout);

        int result = s->valid;
        munmap(map, map_size);
        _exit(result ? 0 : 1);
    }

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    size_t sent = 0;

    while (sent < total) {
        sem_wait(&s->empty);

        size_t amount = total - sent;
        if (amount > CHUNK_SIZE)
            amount = CHUNK_SIZE;

        memset(data, 'A', amount);
        s->chunk_size = amount;
        sent += amount;

        sem_post(&s->full);
    }

    sem_wait(&s->empty);
    s->chunk_size = 0;
    sem_post(&s->full);

    int status;
    waitpid(pid, &status, 0);

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed = (end.tv_sec - start.tv_sec) +
                     (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Shared memory transfer size: %ld MB\n", mb);
    printf("Parent wrote: %zu bytes\n", sent);
    printf("Transfer time: %.6f seconds\n", elapsed);
    printf("Child exit status: %s\n",
           WIFEXITED(status) && WEXITSTATUS(status) == 0
           ? "SUCCESS" : "FAILURE");

    sem_destroy(&s->empty);
    sem_destroy(&s->full);
    munmap(map, map_size);
    shm_unlink(name);

    return 0;
}
