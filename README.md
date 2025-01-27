# Process Scheduling Utilities and FCFS Implementation

## Files:
1. `process_utils.h`:
   - Contains the `Process` structure and function declarations for process creation and sorting.

2. `process_utils.c`:
   - Implements the utility functions for process generation, sorting, and setup.

3. `queue_utils.h`:
   - Contains the `Queue` structure and function declarations for queue creation and queue operations like enqueue and dequeue.
  
4. `queue_utils.c`:
   - Implements the utility functions for queue operations like enqueue and dequeue.

5. `fcfs.c`:
   - Implements the First-Come First-Served (FCFS) scheduling algorithm.

6. `rr.c`:
   - Implements the Round Robin (RR) scheduling algorithm.
  
7. `srtf.c`:
   - Implements the Shortest Remaining Time First (SRTF) scheduling algorithm.
     
8. `sjf.c`:
   - Implements the Shortest Job First (SJF) scheduling algorithm, handling time chart visualization and metrics calculation.

## How to Use:
- Include `process_utils.h` and `queue_utils.h` in your scheduling algorithm file.
- Link `process_utils.c` and `queue_utils.c` when compiling your algorithm.

## Compilation Instructions:
- For FCFS:
  ```bash
  gcc -o fcfs_scheduler fcfs.c process_utils.c
  ./fcfs_scheduler
- For SJF:
  ```bash
  gcc -o sjf_scheduler sjf.c process_utils.c queue_utils.c -lm
  ./sjf_scheduler
- For SRTF:
  ```bash
  gcc -o srtf_scheduler srtf.c process_utils.c queue_utils.c -lm
  ./srtf_scheduler
