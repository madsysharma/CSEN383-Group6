# Process Scheduling Utilities and FCFS Implementation

## Files:
1. `process_utils.h`:
   - Contains the `Process` structure and function declarations for process creation and sorting.

2. `process_utils.c`:
   - Implements the utility functions for process generation, sorting, and setup.

3. `fcfs.c`:
   - Implements the First-Come First-Served (FCFS) scheduling algorithm.

4. `rr.c`:
   - Implements the Round Robin (RR) scheduling algorithm.
  
5. `srtf.c`:
   - Implements the Shortest Remaining Time First (SRTF) scheduling algorithm.

## How to Use:
- Include `process_utils.h` and `queue_utils.h` in your scheduling algorithm file.
- Link `process_utils.c` and `queue_utils.h` when compiling your algorithm.

## Compilation Instructions:
- For FCFS:
  ```bash
  gcc -o fcfs_scheduler fcfs.c process_utils.c
  ./fcfs_scheduler
