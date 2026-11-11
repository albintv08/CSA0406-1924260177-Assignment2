#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define BUFFER_SIZE 65536

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <size_in_MB>\n", argv[0]);
        return 1;
    }

    long mb = atol(argv[1]);
    if (mb <= 0) return 1;

    size_t size = (size_t)mb * 1024 * 1024;

    int fd[2];
    if (pipe(fd) == -1) {
        perror("pipe");
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        close(fd[1]);

        char buffer[BUFFER_SIZE];
        size_t received = 0;
        int valid = 1;
        ssize_t n;

        while ((n = read(fd[0], buffer, sizeof(buffer))) > 0) {
            received += (size_t)n;

            for (ssize_t i = 0; i < n; i++) {
                if (buffer[i] != 0) {
                    valid = 0;
                    break;
                }
            }
        }

        close(fd[0]);

        printf("Child received: %zu bytes\n", received);
        printf("Data validation: %s\n",
               valid && received == size ? "SUCCESS" : "FAILURE");
        fflush(stdout);

        _exit(valid && received == size ? 0 : 1);
    }

    close(fd[0]);

    char buffer[BUFFER_SIZE] = {0};
    size_t sent = 0;
    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    while (sent < size) {
        size_t amount = size - sent;
        if (amount > BUFFER_SIZE) amount = BUFFER_SIZE;

        ssize_t n = write(fd[1], buffer, amount);
        if (n <= 0) {
            perror("write");
            close(fd[1]);
            waitpid(pid, NULL, 0);
            return 1;
        }

        sent += (size_t)n;
    }

    close(fd[1]);

    int status;
    waitpid(pid, &status, 0);

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed = (end.tv_sec - start.tv_sec) +
                     (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Pipe transfer size: %ld MB\n", mb);
    printf("Parent sent: %zu bytes\n", sent);
    printf("Transfer time: %.6f seconds\n", elapsed);
    printf("Child exit status: %s\n",
           WIFEXITED(status) && WEXITSTATUS(status) == 0
           ? "SUCCESS" : "FAILURE");

    return 0;
}
