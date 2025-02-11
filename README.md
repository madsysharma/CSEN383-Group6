# CSEN 383 - Group 6 - Projects
## Project 2 - Process Scheduling

### Files:
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

### How to Use:
- Include `process_utils.h`,`queue_utils.h` and `simulation.h` in your scheduling algorithm file.
- Link all files with `.c` during compilation.

### Compilation Instructions:
  ```bash
  gcc -o main *.c
  ./main
  ```


## Project 3 - Multithreaded Ticket Sellers

### Files:
1. `queue_utils.h`:
   - Contains the `Queue` structure and function declarations for queue creation and queue operations like enqueue and dequeue.
  
2. `queue_utils.c`:
   - Implements the utility functions for queue operations like enqueue and dequeue.
  
3. `main.c`:
    - The main program containing all definitions and declarations for functions involved in the multithreading process for simulating concert ticket selling.

### How to Use:
- Include `queue_utils.h` in the main.c file.
- Link all files with `.c` during compilation.

### Compilation Instructions:
  ```bash
  gcc -o main *.c
  ./main <insert queue size here>
  ```

## Project 4 - Paging and Swapping

### Files (present in the Code folder):
1. `utils.h`:
   - Contains the structures for Queue, Page, Process and Page List, and function declarations for queue creation and queue operations like enqueue and dequeue, as well as process creation, page list initialization, and other page helper functions.
  
2. `utils.c`:
   - Implements the utility functions declared in utils.h
  
3. `main.c`:
    - The main program containing all definitions and declarations for functions involved in the paging and swapping simulation.

4. `helper.h`:
    - Contains function prototypes for simulating page replacement algorithms during process execution.
	
5. `fifo.c`, `lru.c`, `lfu.c`, `randomPick.c` and `mfu.c`:
    - Simulates the FIFO, LRU, LFU, Random Pick and MFU page replacement algorithms respectively.

### How to Use:
- Include `helper.h` in the main.c file.
- Link all files with `.c` during compilation.

### Compilation Instructions:
  ```bash
  gcc -g *.c -o main
  ./main
  ```
