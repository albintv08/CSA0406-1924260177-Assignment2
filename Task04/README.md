# Q4 – Inter-Process Communication (IPC)

## 1. Objective

The objective of this task is to implement and compare two inter-process
communication mechanisms in Linux:

1. Pipe-based communication.
2. Shared-memory communication using process-shared semaphores.

Both implementations transfer data between a parent and child process.
Performance is measured for 1 MB, 10 MB and 100 MB data transfers.

## 2. IPC Overview

Inter-Process Communication (IPC) allows independent processes to exchange
data and coordinate their activities.

### Pipe Communication

A pipe provides a unidirectional byte stream managed by the operating system.
The parent process writes data into the pipe, and the child process reads it.

### Shared-Memory Communication

Shared memory allows two processes to access the same memory region.
The parent writes data into the shared region, and the child reads and
validates it. Process-shared semaphores coordinate access and prevent the
producer from overwriting data before the consumer has finished reading it.

## 3. Architecture

### Pipe

Parent Process
    |
    | write()
    v
Kernel Pipe Buffer
    |
    | read()
    v
Child Process
    |
    v
Byte Validation

### Shared Memory

Parent Process
    |
    | Write data
    v
Shared Memory Region (4 MB reusable chunk)
    |
    | Synchronization using semaphores
    v
Child Process
    |
    | Read and validate
    v
Next chunk / Completion

## 4. Pipe Implementation

Source file: `pipe_ipc.c`

The pipe implementation performs the following operations:

1. Creates a pipe using `pipe()`.
2. Creates a child process using `fork()`.
3. The parent closes its unused read end.
4. The child closes its unused write end.
5. The parent sends zero-filled data in chunks of 64 KB.
6. The child reads the incoming bytes and validates that every byte is zero.
7. The parent closes the write end after completing the transfer.
8. The child receives EOF when all write descriptors are closed and no
   buffered data remains.
9. The parent waits for the child and reports the transfer time.

The child also checks whether the total received byte count matches the
requested transfer size.

### Pipe EOF Behaviour

When the parent closes the pipe's write descriptor, the child eventually
receives a return value of zero from `read()` after all buffered data has
been consumed. This indicates end-of-file (EOF).

## 5. Shared-Memory Implementation

Source file: `shm_ipc.c`

The shared-memory implementation performs the following operations:

1. Creates a shared memory object.
2. Maps the shared region into the address spaces of both processes.
3. Uses a reusable 4 MB shared buffer.
4. Initializes process-shared semaphores named `empty` and `full`.
5. The parent waits for the buffer to become empty before writing.
6. The parent fills the buffer with the data.
7. The parent signals that the buffer is full.
8. The child waits for the full signal, reads the data and validates every byte.
9. The child signals that the buffer is empty so the next chunk can be written.
10. The process repeats until the requested data size is transferred.

The 4 MB reusable buffer allows the program to transfer 100 MB without
requiring one large shared-memory allocation.

### Semaphore Synchronization

- `empty`: Indicates that the shared buffer is available for writing.
- `full`: Indicates that the shared buffer contains data ready for reading.

This producer-consumer synchronization prevents the parent from overwriting
data before the child has consumed it.

## 6. Experimental Results

Both implementations were compiled and executed in the Linux Codespaces
environment. Each test validates the complete transferred data.

### Pipe Measurements

| Transfer Size | Actual Transfer Time | Validation |
|---|---:|---|
| 1 MB | 0.002591 seconds | SUCCESS |
| 10 MB | 0.017676 seconds | SUCCESS |
| 100 MB | 0.166403 seconds | SUCCESS |

### Shared-Memory Measurements

| Transfer Size | Actual Transfer Time | Validation |
|---|---:|---|
| 1 MB | 0.004447 seconds | SUCCESS |
| 10 MB | 0.018358 seconds | SUCCESS |
| 100 MB | 0.155237 seconds | SUCCESS |

### Comparison

| Transfer Size | Pipe (seconds) | Shared Memory (seconds) | Faster Method |
|---|---:|---:|---|
| 1 MB | 0.002591 | 0.004447 | Pipe |
| 10 MB | 0.017676 | 0.018358 | Pipe |
| 100 MB | 0.166403 | 0.155237 | Shared Memory |

## 7. Analysis

### 1 MB Transfer

Pipe communication completed in 0.002591 seconds, while shared memory
completed in 0.004447 seconds. Pipe was faster in this test.

### 10 MB Transfer

Pipe communication completed in 0.017676 seconds, compared with
0.018358 seconds for shared memory. The difference was small.

### 100 MB Transfer

Shared memory completed in 0.155237 seconds, while pipe communication
required 0.166403 seconds. Shared memory was faster in this test.

### Interpretation

Pipes are simple to implement and provide kernel-managed communication.
Shared memory allows processes to access a common memory region, but requires
explicit synchronization.

The results show that neither method was fastest for every transfer size.
For the 100 MB test, shared memory performed better, while pipe communication
was faster for the 1 MB and 10 MB tests.

The implementations also use different chunk sizes: 64 KB for pipe writes
and a reusable 4 MB shared-memory region. Therefore, the measurements reflect
both the IPC mechanism and the implementation choices.

These are single-run measurements from the Codespaces environment. Execution
times can vary depending on system load and available resources. They should
not be treated as universal performance values.

## 8. Advantages and Limitations

### Pipes

Advantages:
- Simple communication interface.
- Kernel manages the communication buffer.
- EOF provides a clear indication that the writer has closed the pipe.

Limitations:
- Data passes through a kernel-managed pipe buffer.
- Communication is unidirectional in this implementation.
- Repeated read and write operations are required.

### Shared Memory

Advantages:
- Both processes access the same mapped memory region.
- Avoids transferring every byte through a pipe buffer.
- Suitable for large data exchange when synchronization is implemented correctly.

Limitations:
- Requires explicit synchronization.
- Incorrect semaphore usage can cause blocking or race conditions.
- Shared-memory allocation and cleanup require additional system calls.

## 9. Conclusion

Both pipe and shared-memory IPC mechanisms were successfully implemented
and tested for 1 MB, 10 MB and 100 MB data transfers.

All six tests completed successfully, and the receiving process validated
the complete data.

Pipe communication was faster in the 1 MB and 10 MB tests. Shared memory
was faster in the 100 MB test. The experiment demonstrates that IPC
performance depends on data size, synchronization overhead, buffer size
and system conditions.

For this experiment, shared memory showed an advantage for the largest
transfer, while pipes provided a simpler solution for smaller transfers.

## 10. Files Included

| File | Description |
|---|---|
| `pipe_ipc.c` | Pipe communication source code |
| `shm_ipc.c` | Shared-memory communication source code |
| `pipe_1mb.txt` | Actual 1 MB pipe output |
| `pipe_10mb.txt` | Actual 10 MB pipe output |
| `pipe_100mb.txt` | Actual 100 MB pipe output |
| `shm_1mb.txt` | Actual 1 MB shared-memory output |
| `shm_10mb.txt` | Actual 10 MB shared-memory output |
| `shm_100mb.txt` | Actual 100 MB shared-memory output |
| `weather_pipe.c` | Weather sensor pipe implementation |
| `weather_output.txt` | Actual weather sensor pipeline output |

## Weather Station Sensor Pipeline – Pipe Implementation

### Objective
A parent process acts as the weather sensor reader, while a child process acts as the data analyser. The parent sends six integer temperature readings through a pipe.

### Implementation
The program uses:
- `pipe()` to create a communication channel.
- `fork()` to create the analyser process.
- `write()` to send sensor readings.
- `read()` to receive readings until the pipe reaches EOF.
- `waitpid()` to wait for the child process.

### Actual Output

    analyser: 6 readings, average 35.5, max 41
    reader: done

### How the Pipe Signals End of Data
After sending all six readings, the parent closes the write end using `close(fd[1])`. Once the child has consumed all buffered data, its next `read()` returns `0`, indicating EOF (end of file). This tells the analyser that no more readings are available.

### IPC Model Comparison

| Feature | Pipe | Shared Memory |
|---|---|---|
| Communication model | Message passing | Shared address space |
| Data transfer | Kernel-managed buffer | Common mapped memory region |
| Data copying | Data is copied between user buffers and the kernel pipe buffer | No per-message kernel-buffer copy is required |
| Synchronization | Blocking read/write and EOF provide basic coordination | Explicit synchronization is required |
| Example used | `read()` and `write()` | Shared memory with semaphores |

Shared memory requires synchronization because the producer and consumer access the same memory region. Without synchronization, the producer could overwrite data before the consumer reads it. The implementation uses process-shared semaphores to coordinate access.

### Files
- `weather_pipe.c` – weather sensor pipe implementation.
- `weather_output.txt` – actual execution output.
