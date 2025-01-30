# CSEN 383 - Group 6 - Projects
# Project 2 - Process Scheduling Utilities and FCFS Implementation

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

9. `simulation.h`:
    - Contains declarations of all scheduling algorithms and their helper functions
  
10. `main.c`:
    - The main program where all the scheduling algorithms are run for 5 trials.

## How to Use:
- Include `process_utils.h`,`queue_utils.h` and `simulation.h` in your scheduling algorithm file.
- Link all files with `.c` during compilation.

## Compilation Instructions:
  ```bash
  gcc -o main *.c
  ./main
