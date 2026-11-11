# Task 05 – Process Synchronization

## 1. Objective

To study process synchronization and mutual exclusion using a railway
ticket-booking scenario. The experiments demonstrate semaphores, mutex
locks, bounded-buffer synchronization, race conditions, and lock granularity.

## 2. Railway Booking Synchronization Design

In a railway reservation system, multiple users may attempt to book seats
at the same time.

Shared resources include:
- Train booking records
- Coach availability
- Seat allocation information
- Pending booking requests

The critical section updates shared booking information. Mutual exclusion
prevents two threads from modifying the same shared data simultaneously.

### Synchronization requirements

**Mutual exclusion:** Only one thread at a time may enter a protected
critical section for the same shared resource.

**Progress:** If no thread is executing in a critical section, the selection
of the next eligible thread should not be postponed indefinitely.

**Bounded waiting:** A thread requesting entry should not wait indefinitely
while other threads repeatedly enter the critical section.

A mutex protects shared booking updates. A production reservation system
should also use transaction-safe database operations and appropriate
fairness policies.

## 3. Peterson's Algorithm

Peterson's algorithm provides mutual exclusion for two processes.

Shared variables:
- `flag[2]`: indicates whether each process wants to enter.
- `turn`: indicates which process should wait when both compete.

Pseudocode:

    flag[i] = true;
    turn = j;

    while (flag[j] && turn == j) {
        // wait
    }

    // critical section

    flag[i] = false;

Peterson's algorithm demonstrates mutual exclusion, progress, and bounded
waiting under its classical assumptions.

### Limitation on modern processors

The original algorithm assumes sequentially consistent memory operations.
Modern CPUs and compilers may reorder memory operations, and cached memory
may not become visible to another thread in the assumed order.

A practical C implementation requires suitable atomic variables and memory
ordering, such as sequentially consistent C11 atomics. Peterson's algorithm
is also limited to two participants and is not a general-purpose scalable
locking mechanism.

## 4. Semaphore-Based Bounded Buffer

The bounded-buffer program models pending railway booking requests.

- Buffer capacity: 5 items
- Items per producer: 20
- Maximum producers: 4
- Maximum consumers: 4

Three synchronization objects are used:

| Object | Initial value | Purpose |
|---|---:|---|
| `empty_slots` | 5 | Counts available buffer positions |
| `full_slots` | 0 | Counts available items |
| `mutex` | Unlocked | Protects shared buffer state |

### Producer operation

1. Wait for an empty buffer position.
2. Acquire the mutex.
3. Insert an item and update the input index.
4. Release the mutex.
5. Signal that an item is available.

### Consumer operation

1. Wait for an available item.
2. Acquire the mutex.
3. Remove an item and update the output index.
4. Record the consumed item and sum.
5. Release the mutex.
6. Signal that a buffer position is available.

The semaphores prevent buffer overflow and underflow. The mutex protects
shared variables such as the buffer indices and consumed-item counter.

## 5. Bounded-Buffer Experimental Results

| Producers | Consumers | Items | Consumed sum | Execution time (s) |
|---:|---:|---:|---:|---:|
| 1 | 1 | 20 | 210 | 0.000784 |
| 4 | 4 | 80 | 3240 | 0.001402 |
| 1 | 4 | 20 | 210 | 0.001252 |
| 4 | 1 | 80 | 3240 | 0.016414 |

All four configurations passed exactly-once verification.

The saved output files contain the final verification results:
- `output_1x1.txt`
- `output_4x4.txt`

## 6. Repeated Reliability Testing

The synchronized implementation was tested 100 times for each of two
configurations.

| Configuration | Successful runs | Failed runs |
|---|---:|---:|
| 1 producer, 1 consumer | 100 | 0 |
| 4 producers, 4 consumers | 100 | 0 |
| Total | 200 | 0 |

The results show that the synchronized implementation maintained
exactly-once consumption in all 200 test runs.

Evidence: `repeated_test.txt`

## 7. Experiment Without Mutex Protection

A separate experimental program, `bounded_buffer_nomutex.c`, was created
by removing mutex protection while retaining the semaphores.

The program was tested with four producers and four consumers for
100 executions.

| Measurement | Result |
|---|---:|
| Total runs | 100 |
| Successful runs | 83 |
| Failed runs | 17 |
| Success rate | 83% |
| Failure rate | 17% |

The failures demonstrate that semaphores controlling buffer availability
do not automatically protect every shared variable.

Concurrent updates to variables such as `next_item`, `in`, `out`, and
`consumed_count` can produce data races.

A successful individual run does not prove that an unsynchronized program
is safe.

Evidence:
- `nomutex_output.txt`
- `nomutex_repeated_test.txt`

## 8. Lock Granularity and Atomic Spinlock Benchmark

A railway booking benchmark was implemented using four threads.
Each thread performed 200,000 booking operations, giving 800,000 total
operations per benchmark run.

Four locking approaches were compared:

1. **Global mutex:** One mutex protects the complete booking array.
2. **Per-train mutex:** Each train has an independent mutex.
3. **Per-coach mutex:** Each coach of each train has its own mutex.
4. **Atomic spinlock:** Threads repeatedly attempt to acquire one
   atomic flag and wait by busy-spinning.

### Five-run execution-time measurements

| Run | Global mutex | Per-train mutex | Per-coach mutex | Atomic spinlock |
|---:|---:|---:|---:|---:|
| 1 | 0.046113 | 0.015393 | 0.009918 | 0.019198 |
| 2 | 0.031593 | 0.018323 | 0.017980 | 0.015580 |
| 3 | 0.025751 | 0.022905 | 0.013536 | 0.009106 |
| 4 | 0.033876 | 0.015913 | 0.013235 | 0.013773 |
| 5 | 0.039959 | 0.017797 | 0.009884 | 0.026290 |

All times are in seconds.

### Average benchmark results

| Method | Average time (s) | Throughput (bookings/s) |
|---|---:|---:|
| Global mutex | 0.035458 | 22,561,650 |
| Per-train mutex | 0.018066 | 44,281,587 |
| Per-coach mutex | 0.012911 | 61,964,587 |
| Atomic spinlock | 0.016789 | 47,649,112 |

Throughput is calculated as 800,000 divided by average execution time.

### Analysis

The global mutex had the highest average execution time because all
booking operations compete for one lock.

Per-train locking reduces contention by allowing operations on different
trains to proceed independently.

Per-coach locking provided the lowest average execution time in this
experiment because independent coaches can be updated concurrently.

The atomic spinlock performed better than the global mutex in the
five-run average, but busy waiting consumes CPU time while a thread
waits for the lock. Spinlocks are most suitable for short critical
sections when waiting times are expected to be very small.

These results are specific to this workload and execution environment.
They should not be treated as universal performance guarantees.

Evidence:
- `lock_benchmark.c`
- `lock_benchmark_output.txt`
- `lock_benchmark_repeated.txt`

## 9. Comparison of Locking Approaches

| Approach | Advantages | Limitations |
|---|---|---|
| Global mutex | Simple, easy to manage | High contention |
| Per-train mutex | Independent trains can operate concurrently | Operations within one train still compete |
| Per-coach mutex | Fine-grained parallelism | More locks and more complex management |
| Atomic spinlock | Avoids sleeping for very short waits | Busy waiting wastes CPU under contention |

## 10. Final Recommendation

For the railway reservation scenario, per-coach locking is a suitable
choice when booking operations affect independent coaches.

A production system must additionally protect seat-level consistency,
prevent double booking, handle cancellations, and use reliable database
transactions.

The experiments demonstrate that synchronization is essential for
protecting shared data. Semaphores manage resource availability, while
mutexes provide mutual exclusion for critical sections.

## 11. Source Code and Evidence Files

| File | Description |
|---|---|
| `bounded_buffer.c` | Semaphore and mutex implementation |
| `bounded_buffer_nomutex.c` | Experimental implementation without mutex |
| `lock_benchmark.c` | Lock granularity and spinlock benchmark |
| `output_1x1.txt` | Single producer/consumer output |
| `output_4x4.txt` | Four producers/consumers output |
| `repeated_test.txt` | 200-run reliability summary |
| `nomutex_output.txt` | Individual no-mutex experiment |
| `nomutex_repeated_test.txt` | 100-run no-mutex summary |
| `lock_benchmark_output.txt` | Initial locking benchmark |
| `lock_benchmark_repeated.txt` | Five repeated locking benchmarks |

## 12. Conclusion

The bounded-buffer implementation successfully synchronized concurrent
producers and consumers in all 200 repeated tests.

Removing mutex protection resulted in 17 failures across 100 executions,
demonstrating the danger of race conditions.

The locking benchmark showed that finer-grained per-coach locking achieved
the best average performance for the tested workload.

Therefore, correct synchronization and appropriate lock granularity are
important for reliable and efficient concurrent systems.

## Railway Booking: Lock Granularity Benchmark

### Objective
The benchmark compares four synchronization methods for a railway
booking workload: global mutex, per-train mutex, per-coach mutex,
and an atomic spinlock.

### Experimental Configuration
- Threads: 4
- Bookings per thread: 200,000
- Total expected bookings per run: 800,000
- Repeated runs: 5 per locking method
- Timer: CLOCK_MONOTONIC
- Verification: total booking count compared with expected count

### Five-Run Average Results

| Locking Method | Average Time (seconds) | Average Throughput (bookings/second) |
|---|---:|---:|
| Global Mutex | 0.035458 | 22,561,650 |
| Per-Train Mutex | 0.018066 | 44,281,587 |
| Per-Coach Mutex | 0.012911 | 61,964,587 |
| Atomic Spinlock | 0.016789 | 47,649,112 |

All five runs of every method completed 800,000 bookings and passed
the verification check.

### Analysis
The per-coach mutex achieved the lowest average execution time and
highest average throughput in this experiment. It protects individual
train-coach booking counters, allowing operations on different coaches
to proceed concurrently.

The per-train mutex provides more concurrency than a global mutex,
because different trains can be updated independently. However,
operations on different coaches of the same train still share a lock.

The global mutex protects the entire booking data structure with one
lock. This provides simple mutual exclusion but creates greater lock
contention when several threads attempt bookings simultaneously.

The atomic spinlock also provides mutual exclusion. However, threads
waiting for the lock repeatedly check its state, consuming CPU time
while waiting. It may be useful for very short critical sections, but
busy waiting can reduce efficiency under contention.

### Conclusion
For this simulated railway booking workload, the per-coach mutex
provided the best average performance across five runs. Fine-grained
locking reduced unnecessary contention while preserving correct
updates. In a real railway reservation system, additional validation
would be needed for seat availability, transaction consistency,
cancellation, and concurrent requests.

### Benchmark Evidence Files
- lock_benchmark.c - Benchmark source code
- lock_benchmark_output.txt - Initial benchmark output
- lock_benchmark_repeated.txt - Five repeated benchmark runs
