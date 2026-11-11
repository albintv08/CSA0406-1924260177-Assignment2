# Q2 – CPU Scheduling Analysis

## 1. Problem Overview

A cloud video-encoding service processes short jobs and long jobs on
the same servers. The objective is to compare CPU scheduling algorithms
and identify a suitable policy for different job lengths.

## 2. Given Process Information

| Process | Arrival Time | Burst Time |
|---------|--------------|------------|
| P1      | 0            | 8          |
| P2      | 1            | 4          |
| P3      | 2            | 9          |
| P4      | 3            | 5          |

Turnaround Time (TAT) = Completion Time - Arrival Time

Waiting Time (WT) = Turnaround Time - Burst Time

Response Time (RT) = First CPU Start Time - Arrival Time

## 3. FCFS – First Come, First Served

### Gantt Chart

P1 (0-8) | P2 (8-12) | P3 (12-21) | P4 (21-26)

| Process | CT | TAT | WT | RT |
|---------|----|-----|----|----|
| P1 | 8  | 8  | 0  | 0  |
| P2 | 12 | 11 | 7  | 7  |
| P3 | 21 | 19 | 10 | 10 |
| P4 | 26 | 23 | 18 | 18 |

Average TAT = 15.25

Average WT = 8.75

Average RT = 8.75

FCFS executes processes in arrival order. It is simple but may cause
short jobs to wait behind long jobs.

## 4. SJF – Shortest Job First (Non-preemptive)

P1 executes first because it is the only process available at time 0.
After P1 completes, the shortest available process is selected.

### Gantt Chart

P1 (0-8) | P2 (8-12) | P4 (12-17) | P3 (17-26)

| Process | CT | TAT | WT | RT |
|---------|----|-----|----|----|
| P1 | 8  | 8  | 0  | 0  |
| P2 | 12 | 11 | 7  | 7  |
| P3 | 26 | 24 | 15 | 15 |
| P4 | 17 | 14 | 9  | 9  |

Average TAT = 14.25

Average WT = 7.75

Average RT = 7.75

SJF reduces average waiting time but may cause starvation of long jobs
if shorter jobs continuously arrive.

## 5. SRTF – Shortest Remaining Time First

SRTF is the preemptive version of SJF. The process with the shortest
remaining CPU burst is selected whenever scheduling occurs.

### Gantt Chart

P1 (0-1) | P2 (1-5) | P4 (5-10) | P1 (10-17) | P3 (17-26)

| Process | CT | TAT | WT | RT |
|---------|----|-----|----|----|
| P1 | 17 | 17 | 9  | 0  |
| P2 | 5  | 4  | 0  | 0  |
| P3 | 26 | 24 | 15 | 15 |
| P4 | 10 | 7  | 2  | 2  |

Average TAT = 13.00

Average WT = 6.50

Average RT = 4.25

SRTF gives the lowest average turnaround and waiting times for this
workload. However, long processes may experience starvation.

## 6. Priority Scheduling (Non-preemptive)

The assignment does not provide priority values. Therefore, the
following values are assumed for comparison. A smaller number means
higher priority.

| Process | Arrival | Burst | Priority |
|---------|---------|-------|----------|
| P1      | 0       | 8     | 4        |
| P2      | 1       | 4     | 1        |
| P3      | 2       | 9     | 3        |
| P4      | 3       | 5     | 2        |

P1 executes first because it is the only process available at time 0.
The remaining processes execute in priority order.

### Gantt Chart

P1 (0-8) | P2 (8-12) | P4 (12-17) | P3 (17-26)

| Process | CT | TAT | WT | RT |
|---------|----|-----|----|----|
| P1 | 8  | 8  | 0  | 0  |
| P2 | 12 | 11 | 7  | 7  |
| P3 | 26 | 24 | 15 | 15 |
| P4 | 17 | 14 | 9  | 9  |

Average TAT = 14.25

Average WT = 7.75

Average RT = 7.75

### Starvation and Ageing

Low-priority processes may wait indefinitely when higher-priority
processes continuously arrive. Ageing gradually increases the priority
of waiting processes, ensuring that they eventually receive CPU time.

## 7. Round Robin Scheduling

Round Robin allocates a fixed time quantum to each ready process.
Unfinished processes return to the end of the ready queue.

The implementation is provided in `round_robin.c`. It accepts the
quantum as a command-line argument and calculates completion,
turnaround, waiting and response times.

### Round Robin with Quantum = 3

Gantt Chart:

P1 (0-3) | P2 (3-6) | P3 (6-9) | P4 (9-12) |
P1 (12-15) | P2 (15-16) | P3 (16-19) |
P4 (19-21) | P1 (21-23) | P3 (23-26)

| Process | CT | TAT | WT | RT |
|---------|----|-----|----|----|
| P1 | 23 | 23 | 15 | 0 |
| P2 | 16 | 15 | 11 | 2 |
| P3 | 26 | 24 | 15 | 4 |
| P4 | 21 | 18 | 13 | 6 |

Average TAT = 20.00

Average WT = 13.50

Average RT = 3.00

## 8. Time Quantum Experiment

The program was executed using four different quantum values.

Context switches count only transitions from one process to a different
process. Consecutive CPU dispatches of the same process are not counted
as context switches. Therefore, the context-switch count may be lower
than the number of dispatches minus one.

| Quantum | Average TAT | Average WT | Average RT | Dispatches | Context Switches |
|---------|-------------|------------|------------|------------|------------------|
| 1  | 19.00 | 12.50 | 0.75 | 26 | 23 |
| 3  | 20.00 | 13.50 | 3.00 | 10 | 9 |
| 5  | 17.50 | 11.00 | 5.50 | 6 | 5 |
| 20 | 15.25 | 8.75 | 8.75 | 4 | 3 |
### Analysis

A smaller quantum improves response time but increases the number
of dispatches and context switches. A larger quantum reduces
scheduling overhead, but processes may wait longer before receiving
their first CPU allocation.

For this workload, quantum 20 produces the lowest average waiting
and turnaround times. However, it behaves like FCFS because every
process finishes within one time slice.

## 9. Multilevel Feedback Queue (MLFQ)

MLFQ uses multiple priority queues with different time quanta.

Proposed design:

- Queue 1: High priority, quantum = 2 time units.
- Queue 2: Medium priority, quantum = 4 time units.
- Queue 3: Low priority, quantum = 8 time units.

New processes enter the highest-priority queue. Processes that use
their entire time quantum without completing are moved to a lower
queue. The scheduler selects from the highest-priority non-empty queue.

Short jobs can finish quickly in the higher-priority queue. Long
encoding jobs move to lower queues and receive longer time slices.

Periodic priority boosts or ageing prevent starvation.

These queue settings are a proposed design; no separate MLFQ
execution or measured averages were provided.

### Scheduling Complexity

| Algorithm | Simplified scheduling cost |
|-----------|----------------------------|
| FCFS | O(1) queue operations |
| SJF | O(n) selection by scanning |
| SRTF | O(n) selection by scanning |
| Priority | O(n) selection by scanning |
| Round Robin | O(1) queue operations |
| MLFQ | O(1) queue operations with a fixed number of queues |

Actual implementation cost depends on the data structure and
number of queues.

## 10. Algorithm Comparison

| Algorithm | Average TAT | Average WT | Average RT |
|-----------|-------------|------------|------------|
| FCFS | 15.25 | 8.75 | 8.75 |
| SJF | 14.25 | 7.75 | 7.75 |
| SRTF | 13.00 | 6.50 | 4.25 |
| Priority (assumed) | 14.25 | 7.75 | 7.75 |
| RR (quantum 3) | 20.00 | 13.50 | 3.00 |

## 11. Final Recommendation

SRTF gives the best average turnaround and waiting times for the
four supplied processes. Round Robin with a small quantum provides
better response time but increases scheduling overhead.

For a real cloud video-encoding service with both short and long jobs,
MLFQ with ageing is recommended. It prioritizes short jobs, allows
long jobs to continue receiving CPU time and reduces starvation risk.

SRTF is the best result for the supplied numerical workload, while
MLFQ is the proposed practical design for the mixed-job service.

## 12. Files and Evidence

- `round_robin.c` – Round Robin source code.
- `output.txt` – Original quantum-3 execution output.
- `quantum_1.txt` – Quantum-1 experiment.
- `quantum_3.txt` – Quantum-3 experiment.
- `quantum_5.txt` – Quantum-5 experiment.
- `quantum_20.txt` – Quantum-20 experiment.

Screenshots of source code, compilation and execution should be
captured after all assignment tasks are completed.

## 13. Conclusion

The experiment demonstrates that CPU scheduling performance depends
on the workload and scheduling policy. SRTF minimizes average waiting
and turnaround times in the supplied example, while Round Robin
provides configurable responsiveness. MLFQ with ageing is a suitable
design for a cloud service that must handle both short and long jobs.
