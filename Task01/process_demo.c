./process_demo | tee output.txt
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define CHILDREN 3

int main(void)
{
    pid_t pid[CHILDREN];

    printf("=== PROCESS MANAGEMENT DEMONSTRATION ===\n");
    printf("Parent Process ID: %d\n", getpid());
    fflush(stdout);

    for (int i = 0; i < CHILDREN; i++) {
        pid[i] = fork();

        if (pid[i] < 0) {
            perror("fork failed");
            return 1;
        }

        if (pid[i] == 0) {
            printf("Child %d created | PID: %d | PPID: %d\n",
                   i + 1, getpid(), getppid());
            fflush(stdout);

            sleep(20);

            printf("Child %d terminating | PID: %d\n",
                   i + 1, getpid());
            fflush(stdout);

            _exit(0);
        }
    }

    printf("\nParent is waiting for child processes...\n");
    printf("Child PIDs: %d %d %d\n", pid[0], pid[1], pid[2]);
    fflush(stdout);

    for (int i = 0; i < CHILDREN; i++) {
        waitpid(pid[i], NULL, 0);
    }

    printf("All child processes completed.\n");

    return 0;
}
