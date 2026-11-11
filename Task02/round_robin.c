#include <stdio.h>
#include <stdlib.h>
#define N 4

int main(int argc, char *argv[])
{
    int quantum = (argc > 1) ? atoi(argv[1]) : 3;
    int slices = 0;
    int arrival[N] = {0, 1, 2, 3};
    int burst[N] = {8, 4, 9, 5};
    int remaining[N];

    int completion[N] = {0};
    int turnaround[N] = {0};
    int waiting[N] = {0};
    int response[N];

    int time = 0;
    int completed = 0;
    int queue[100];
    int front = 0, rear = 0;
    int in_queue[N] = {0};

    for (int i = 0; i < N; i++) {
        remaining[i] = burst[i];
        response[i] = -1;
    }

    printf("ROUND ROBIN SCHEDULING (Quantum = %d)\n", quantum);
    printf("Gantt Chart:\n");

    while (completed < N) {

        for (int i = 0; i < N; i++) {
            if (arrival[i] <= time && !in_queue[i] && remaining[i] > 0) {
                queue[rear++] = i;
                in_queue[i] = 1;
            }
        }

        if (front == rear) {
            time++;
            continue;
        }

        int p = queue[front++];
slices++;
        int start = time;

        if (response[p] == -1)
            response[p] = time - arrival[p];

        int run = remaining[p] < quantum ? remaining[p] : quantum;

        time += run;
        remaining[p] -= run;

        printf("P%d(%d-%d) ", p + 1, start, time);

        for (int i = 0; i < N; i++) {
            if (arrival[i] > start && arrival[i] <= time &&
                !in_queue[i] && remaining[i] > 0) {
                queue[rear++] = i;
                in_queue[i] = 1;
            }
        }

        if (remaining[p] > 0) {
            queue[rear++] = p;
        } else {
            completion[p] = time;
            completed++;
        }
    }

    double total_wt = 0, total_tat = 0, total_rt = 0;

    printf("\n\nPROCESS\tAT\tBT\tCT\tTAT\tWT\tRT\n");

    for (int i = 0; i < N; i++) {
        turnaround[i] = completion[i] - arrival[i];
        waiting[i] = turnaround[i] - burst[i];

        total_wt += waiting[i];
        total_tat += turnaround[i];
        total_rt += response[i];

        printf("P%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
               i + 1, arrival[i], burst[i], completion[i],
               turnaround[i], waiting[i], response[i]);
    }

    printf("\nAverage Turnaround Time: %.2f\n", total_tat / N);
    printf("Average Waiting Time: %.2f\n", total_wt / N);
    printf("Average Response Time: %.2f\n", total_rt / N);
printf("CPU Dispatches: %d\n", slices);
printf("Context Switches (between dispatches): %d\n",
       slices > 0 ? slices - 1 : 0);

    return 0;
}
