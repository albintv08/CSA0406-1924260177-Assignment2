# Q3 – Multithreading for Traffic Camera Plate Recognition

## 1. Problem Overview

A traffic monitoring system receives data from multiple cameras. Each camera
generates vehicle images that must be processed for number-plate recognition.

Creating a separate thread for every camera is one possible approach.
Alternatively, a fixed-size thread pool can process camera jobs using a
controlled number of worker threads.

This experiment compares one-thread-per-camera execution with fixed thread
pools containing 4, 8 and 16 workers.

## 2. Experimental Configuration

| Parameter | Value |
|-----------|-------|
| Number of cameras | 16 |
| Work per camera | 5,000,000 iterations |
| One-thread-per-camera model | 16 threads |
| Thread pool configurations | 4, 8 and 16 workers |
| Programming language | C |
| Thread library | POSIX pthreads |
| Timing method | CLOCK_MONOTONIC |
| Repeated measurements | 5 runs |

The program uses a computational workload to simulate camera processing.
It is a thread-performance benchmark, not a real image-recognition system.

## 3. Implementation

Source file: `thread_benchmark.c`

The program implements two execution models:

### A. One Thread Per Camera

- A separate POSIX thread is created for each of the 16 cameras.
- Each thread processes its assigned camera workload.
- The main thread waits for all camera threads using `pthread_join()`.

### B. Fixed Thread Pool

Three worker-pool configurations are tested:

- 4 worker threads
- 8 worker threads
- 16 worker threads

The pool maintains a shared task index. Each worker obtains the next available
camera task while holding a mutex lock. The lock is released before the worker
processes the task.

This allows multiple workers to process different cameras concurrently.

## 4. Five-Run Experimental Results

The benchmark was executed five times with 5,000,000 iterations per camera.

| Run | One thread per camera (s) | 4 workers (s) | 8 workers (s) | 16 workers (s) |
|-----|---------------------------:|--------------:|--------------:|---------------:|
| 1 | 0.149880 | 0.155572 | 0.156005 | 0.178154 |
| 2 | 0.173171 | 0.139488 | 0.187270 | 0.183396 |
| 3 | 0.145086 | 0.146760 | 0.144489 | 0.161330 |
| 4 | 0.190568 | 0.155963 | 0.155341 | 0.141977 |
| 5 | 0.167141 | 0.186232 | 0.138804 | 0.139248 |
| **Average** | **0.165169** | **0.156803** | **0.156382** | **0.160821** |

## 5. Average Speedup Calculation

The one-thread-per-camera average execution time is used as the baseline.

Speedup = Baseline execution time / Configuration execution time

| Configuration | Average time (s) | Average speedup |
|----------------|-----------------:|----------------:|
| One thread per camera | 0.165169 | 1.00x |
| Fixed pool – 4 workers | 0.156803 | 1.05x |
| Fixed pool – 8 workers | 0.156382 | 1.06x |
| Fixed pool – 16 workers | 0.160821 | 1.03x |

The 8-worker pool achieved the lowest average execution time.
Its speedup was approximately 1.06x compared with the baseline.

The difference between the 4-worker and 8-worker configurations is small.
Therefore, the results do not indicate a major performance advantage for
one configuration over the other.

## 6. Analysis of Results

The experiment demonstrates that increasing the number of threads does not
always improve performance.

The 4-worker and 8-worker pools performed slightly better on average than
the one-thread-per-camera model. The 16-worker pool was slightly slower than
the baseline.

Possible reasons include:

- Thread creation and management overhead.
- CPU scheduling and context-switch overhead.
- Competition for available CPU cores.
- Mutex synchronization when workers obtain tasks.
- Variations in shared Codespaces system load.

The execution times are short and vary between runs. The results should
therefore be interpreted as measurements from this test environment rather
than universal performance guarantees.

## 7. Shared and Private Thread Resources

### Shared Resources

Threads belonging to the same process normally share:

- Process address space.
- Global and static variables.
- Heap memory.
- Open file descriptors.
- Process resources.

In this implementation, the worker threads share the task array and the
next-task index. A mutex protects access to the shared task index.

### Private Resources

Each thread maintains its own:

- Stack.
- CPU register state.
- Program counter.
- Thread-local storage.

These private resources allow each worker to execute independently.

## 8. User-Level and Kernel-Level Threads

### User-Level Threads

User-level thread management is performed by a user-space library.
Thread operations can be lightweight, but in a many-to-one mapping,
a blocking system call can block the entire process and multiple threads
may not execute simultaneously on separate CPU cores.

### Kernel-Level Threads

The operating-system kernel manages and schedules kernel threads.
They can execute in parallel on multiple CPU cores. However, thread
creation, scheduling and synchronization involve operating-system overhead.

### Mapping Models

| Mapping model | Description |
|---------------|-------------|
| Many-to-One | Many user threads map to one kernel thread. |
| One-to-One | Each user thread maps to a kernel thread. |
| Many-to-Many | Many user threads are multiplexed over multiple kernel threads. |

POSIX pthreads on typical Linux systems use kernel-supported threads.

## 9. Processes Versus Threads

| Feature | Processes | Threads |
|---------|-----------|---------|
| Address space | Separate | Shared within a process |
| Communication | IPC mechanisms required | Shared memory is available |
| Creation overhead | Generally higher | Generally lower |
| Context switching | Generally more expensive | Generally less expensive |
| Fault isolation | Stronger | Weaker |
| Synchronization | IPC or shared-memory mechanisms | Mutexes, condition variables, etc. |

For a traffic-camera service, threads are useful when workers need to share
configuration, task queues and application data. Separate processes provide
stronger isolation when camera services must be protected from failures in
other services.

## 10. Recommendation

A fixed-size thread pool is recommended for a multi-camera monitoring
application because it controls the number of concurrent workers and
avoids creating an unlimited number of threads as camera jobs arrive.

In this experiment, the 8-worker pool achieved the best five-run average.
However, the 4-worker pool had nearly the same average performance and uses
fewer workers. The appropriate pool size should be selected through further
testing on the actual deployment hardware and expected camera workload.

For a production plate-recognition system, the application should also use
real image-processing tasks, bounded task queues, error handling and
appropriate synchronization.

## 11. Files and Evidence

| File | Purpose |
|------|---------|
| `thread_benchmark.c` | POSIX multithreading implementation |
| `thread_benchmark` | Compiled executable |
| `output.txt` | Final single-run output |
| `repeated_runs.txt` | Initial five-run results with 500,000 iterations |
| `large_workload_runs.txt` | Five-run results with 5,000,000 iterations |
| `README.md` | Q3 explanation, measurements and analysis |

## 12. Conclusion

The experiment compared one-thread-per-camera execution with fixed thread
pools of 4, 8 and 16 workers.

The five-run measurements showed that the 8-worker pool achieved the best
average execution time of 0.156382 seconds, with an average speedup of
approximately 1.06x over the baseline.

The experiment also demonstrated that adding more threads does not
automatically improve performance. A suitable thread-pool size must balance
parallel execution, synchronization overhead and available CPU resources.
