# Q6 – Deadlock Handling and Banker's Algorithm

## 1. Objective

This task studies deadlocks in operating systems and implements the
Banker's Algorithm for deadlock avoidance. The implementation checks
whether a system is in a safe state, evaluates resource requests and
measures the safety algorithm for different process counts.

## 2. Deadlock and Its Four Necessary Conditions

A deadlock occurs when a group of processes becomes permanently
blocked because each process is waiting for a resource held by
another process.

Four conditions must hold simultaneously for a deadlock to occur:

1. **Mutual Exclusion:** At least one resource is non-shareable and
   can be used by only one process at a time.
2. **Hold and Wait:** A process holds one or more resources while
   waiting for additional resources.
3. **No Preemption:** Resources cannot be forcibly taken away; they
   must be released voluntarily by the process holding them.
4. **Circular Wait:** A circular chain exists in which each process
   waits for a resource held by the next process.

Breaking at least one of these conditions can prevent deadlock.

## 3. Resource Allocation Graph

A Resource Allocation Graph (RAG) represents processes and resources.

- A process is represented by a circle.
- A resource is represented by a rectangle.
- Resource -> Process represents an allocated resource.
- Process -> Resource represents a pending request.

### Example of circular wait

```text
[R1] ---> (P1) ---> [R2]
 ^                     |
 |                     v
 +------- (P2) <-------+
gcc -std=c11 -Wall -Wextra -o bankers bankers.c
Interpretation:
- P1 holds R1 and requests R2.
- P2 holds R2 and requests R1.
- Neither process can continue until the other releases its resource.
For single-instance resource types, a cycle in the resource allocation
graph indicates deadlock. With multiple instances, a cycle alone is
not sufficient to prove deadlock.
4. Deadlock Handling Strategies
4.1 Deadlock Prevention
Prevention ensures that at least one of the four necessary conditions
cannot hold.

Examples:
- Make selected resources shareable where possible.
- Require processes to request resources in a fixed order.
- Require processes to request resources before execution.
- Allow resource preemption where appropriate.
Advantage: Deadlock is prevented by design.

Limitation: Resource utilization and concurrency may decrease.
4.2 Deadlock Avoidance
Avoidance examines resource requests before granting them. A request
is granted only if the resulting system state remains safe.

The Banker's Algorithm is a deadlock-avoidance technique. It requires
maximum resource demands to be declared in advance.

Advantage: Allows resource allocation while maintaining a safe state.

Limitation: Requires advance knowledge of maximum resource needs and
adds checking overhead.
4.3 Deadlock Detection and Recovery
The operating system allows resource allocation and periodically
checks whether deadlock has occurred.

Recovery methods include:
- Terminating one or more processes.
- Preempting resources where possible.
- Rolling back processes to a previous safe checkpoint.
Advantage: Resources can be allocated without restrictive avoidance
checks on every request.

Limitation: Detection and recovery can be expensive and may interrupt
important work.
4.4 Ostrich Approach
The ostrich approach ignores the possibility of deadlock when its
probability and impact are considered sufficiently low.

Advantage: No additional prevention or detection overhead.

Limitation: If a deadlock occurs, manual intervention or a restart
may be required.
5. Comparison of Deadlock Strategies
Strategy	Main Method	Advantage	Limitation
Prevention	Break a necessary condition	Prevents deadlock	Can restrict utilization
Avoidance	Grant only safe requests	Maintains safe states	Requires maximum-demand information
Detection and Recovery	Detect and recover after deadlock	Flexible allocation	Recovery can be costly
Ostrich	Ignore deadlocks	Minimal overhead	Deadlocks remain possible

6. Banker's Algorithm
6.1 Initial System Data
Number of processes: 5

Number of resource types: 3

Available vector:
Resource	A	B	C
Available	3	3	2

Maximum demand matrix:
Process	A	B	C
P0	7	5	3
P1	3	2	2
P2	9	0	2
P3	2	2	2
P4	4	3	3

Allocation matrix:
Process	A	B	C
P0	0	1	0
P1	2	0	0
P2	3	0	2
P3	2	1	1
P4	0	0	2

6.2 Need Matrix
The Need matrix is calculated using:

Need = Maximum - Allocation
Process	A	B	C
P0	7	4	3
P1	1	2	2
P2	6	0	0
P3	0	1	1
P4	4	3	1

6.3 Safety Algorithm
1. Initialize Work = Available.
2. Set Finish[i] = false for every process.
3. Find an unfinished process whose Need is less than or equal
   to Work for every resource type.
4. If found, add its Allocation to Work and mark it finished.
5. Add the process to the safe sequence.
6. Repeat until all processes finish or no eligible process exists.
7. If every process finishes, the state is safe. Otherwise, it is unsafe.
6.4 Initial Safety Result
The program produced:

System is SAFE.

Safe sequence:

P1 -> P3 -> P4 -> P0 -> P2

This sequence demonstrates that all five processes can complete
without entering a deadlock state.
7. Resource Request Evaluation
The program checks that a request:
- Does not exceed the process's remaining Need.
- Does not exceed currently Available resources.
- Leaves the system in a safe state after tentative allocation.
If the tentative allocation is unsafe, the program rolls back the
allocation and denies the request.
Actual request results
Process	Request	Decision	Explanation
P1	(1, 0, 2)	Granted	Resources are available and the resulting state is safe
P4	(3, 3, 0)	Denied	Tentative allocation produces an unsafe state
P0	(0, 2, 0)	Granted	The resulting state has a safe sequence

For P1, the safe sequence is:

P1 -> P3 -> P4 -> P0 -> P2

For P0, the safe sequence after the request is:

P3 -> P1 -> P2 -> P0 -> P4

Note: The P0 request is granted because the implementation
successfully finds a safe sequence. The decision is based on the
provided resource matrices and the algorithm's actual result.
8. Performance Measurement
The safety algorithm was tested with synthetic workloads containing
10, 100 and 1,000 processes. Each process was assigned three resource
types. The workload was constructed so that processes could finish
progressively as resources became available.
Actual measurements
Number of Processes	Completed Processes	Safe State	Execution Time
10	10	Yes	0.000001292 seconds
100	100	Yes	0.000039634 seconds
1,000	1,000	Yes	0.003798717 seconds

All processes completed successfully in the three performance tests.

These measurements are from individual runs in the Codespaces
environment. Execution times can vary with system load and scheduling.
Complexity Analysis
Let:
- n = number of processes
- m = number of resource types
The safety algorithm may scan all n processes repeatedly. Each scan
checks up to m resource types.

Therefore, the worst-case time complexity is:

O(m * n^2)

For this experiment, m = 3. The memory required for the matrices and
working arrays is O(m * n).

The measured time increases with the number of processes because
the algorithm performs more process scans and resource comparisons.
9. Implementation Details
Source file: bankers.c

The program contains:
- Need matrix calculation.
- Banker's safety algorithm.
- Safe-sequence display.
- Resource-request validation.
- Tentative allocation and rollback.
- Performance testing for 10, 100 and 1,000 processes.






## 10. Compilation and Execution

Compile:

    gcc -std=c11 -Wall -Wextra -o bankers bankers.c

Run:

    ./bankers

Save the output:

    ./bankers | tee bankers_output.txt

## 11. Files and Evidence

| File | Purpose |
|---|---|
| bankers.c | Banker's Algorithm implementation |
| bankers_output.txt | Actual program output and performance measurements |
| README.md | Theory, calculations, results and analysis |

## 12. Conclusion

The implementation demonstrates how Banker's Algorithm avoids
deadlock by granting a resource request only when the resulting
system state remains safe.

The initial system was safe with the sequence
P1 -> P3 -> P4 -> P0 -> P2. The request tests demonstrated both
successful allocation and denial of an unsafe tentative allocation.
The performance experiment completed all processes for workloads
of 10, 100 and 1,000 processes.

The experiment supports the use of deadlock avoidance in systems
where maximum resource requirements are known in advance.
