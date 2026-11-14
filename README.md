# CSA0406 – Operating Systems Assignment 2

**Course:** Operating Systems  
**Assignment:** Assignment 2 – CO2  
**Student:** ALBIN T.V.  
**Register Number:** 1924260177  
**Department:** AIDS  
**Academic Year:** 2026

## Assignment Objective

To analyse process management and deadlock-handling techniques and evaluate how they influence CPU utilisation, synchronisation, resource allocation, and overall system performance.

## Repository Structure

| Folder | Task | Main Contents |
|---|---|---|
| `Task01/` | Process Management and Process States | Process creation using `fork()` and process inspection |
| `Task02/` | CPU Scheduling | Round Robin implementation and scheduling outputs |
| `Task03/` | Threads | Multithreaded traffic-camera benchmark |
| `Task04/` | Inter-Process Communication | Pipe and shared-memory implementations |
| `Task05/` | Synchronisation | Bounded-buffer implementation and locking benchmarks |
| `Task06/` | Deadlock Handling | Banker's Algorithm implementation and performance results |
| `screenshots/` | Evidence Images | Assignment execution screenshots, when added |

## Programming Language and Environment

- **Programming language:** C
- **Operating system:** Linux (GitHub Codespaces)
- **Compiler:** GCC
- **Version control:** Git and GitHub
- **Threading and synchronisation:** POSIX threads, mutexes and semaphores

## Compilation and Execution

Programs can be compiled using GCC from the repository root.

Examples:

```bash
gcc Task01/process_demo.c -o Task01/process_demo
./Task01/process_demo

gcc Task02/round_robin.c -o Task02/round_robin
./Task02/round_robin

gcc -pthread Task03/thread_benchmark.c -o Task03/thread_benchmark
./Task03/thread_benchmark

gcc Task04/pipe_ipc.c -o Task04/pipe_ipc
./Task04/pipe_ipc

gcc -pthread Task04/shm_ipc.c -o Task04/shm_ipc
./Task04/shm_ipc

gcc -pthread Task05/bounded_buffer.c -o Task05/bounded_buffer
./Task05/bounded_buffer

gcc -pthread Task05/lock_benchmark.c -o Task05/lock_benchmark
./Task05/lock_benchmark

gcc Task06/bankers.c -o Task06/bankers
./Task06/bankers
```

Some programs may require additional POSIX libraries or platform-specific support depending on the environment.

## Evidence and Results

Each task folder contains its relevant C source files, README documentation, and recorded execution outputs or benchmark results.

The recorded results are used in the assignment report to explain implementation behaviour, compare approaches, and justify design decisions.

## Version Control

The repository uses Git for source-code management. Compiled executables and temporary object files are excluded using `.gitignore`.

## Conclusion

This repository contains the implementations, experiments, and recorded outputs for all six tasks in Operating Systems Assignment 2. It supports the accompanying academic report and demonstrates practical applications of operating-system concepts.
