# Q1 – Process Management for an Online Examination Server

## 1. Problem Overview

The online examination server must support approximately 2,000 concurrent student sessions. Creating a separate process for every student can increase memory consumption and context-switching overhead.

## 2. Process-State Diagram

    NEW --admitted--> READY --dispatch--> RUNNING
                         ^                  |
                         |                  |
                  I/O complete       I/O request
                         |                  v
                      WAITING <-------------+

    RUNNING --time slice expired--> READY
    RUNNING --exit---------------> TERMINATED

A process moves between states depending on CPU scheduling, I/O requests and completion.

## 3. Process Control Block (PCB)

A Process Control Block stores the information the operating system needs to manage and resume a process.

Important PCB fields:

- Process ID (PID)
- Process state
- Program counter
- CPU registers
- CPU scheduling information
- Memory-management information
- I/O status and open-file information
- Parent process ID and accounting information

## 4. Fork System Call Implementation

The program `process_demo.c` uses `fork()` to create three child processes. Each child displays its PID and parent PID, sleeps for 20 seconds and terminates. The parent uses `waitpid()` to wait for all children.

### Actual execution evidence

See `output.txt` for the program output.

See `ps_output.txt` for the process snapshot captured while the parent and three children were active.

The process snapshot showed:

- Parent PID: 14783
- Child PIDs: 14785, 14786 and 14787
- All three children had PPID 14783.
- All four processes were in sleeping state (S) when observed.

## 5. Evaluation of 2,000 Concurrent Processes

Creating 2,000 separate processes can cause:

- Increased memory usage because each process has its own virtual address space and process-management structures.
- Context-switching overhead when the CPU switches between runnable processes.
- Scheduling overhead from managing a large number of processes.
- Additional resource use when processes wait for I/O.

The actual memory requirement depends on the program, shared pages, operating-system overhead and workload. It should be measured on the target server rather than assumed to be a fixed value.

## 6. Execution Model Comparison

### One process per student

Each student session runs in a separate process. This provides strong isolation, but uses more memory and can increase process-management overhead.

### One thread per student

Threads share the server process address space and resources. They generally require less memory than separate processes and can communicate efficiently. However, shared data requires synchronization, and an unhandled process-wide failure can affect all sessions.

### Fixed pool of worker processes

A fixed number of worker processes handle student requests. This limits the number of active workers, controls resource usage and provides process-level isolation between workers. Requests can wait in a queue when all workers are busy.

## 7. Recommended Design

A fixed pool of worker processes is recommended for the examination server, combined with a request queue and suitable load balancing. The pool size should be selected using workload testing and server capacity measurements.

A thread pool inside each worker can also be considered for handling concurrent I/O operations. This hybrid design balances resource efficiency and fault isolation.

## 8. Conclusion

The fork demonstration confirms process creation, parent-child relationships and process termination using `waitpid()`. The `ps_output.txt` snapshot provides evidence that the parent and child processes were active simultaneously. For a large examination workload, a controlled worker pool is more practical than creating an unrestricted process for every student.
