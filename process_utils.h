#ifndef PROCESS_UTILS_H
#define PROCESS_UTILS_H

#define MAX_PROCESSES 100

// Define the Process structure
typedef struct Process {
    char name;         // Process name (e.g., A, B, C...)
    int arrivalTime;   // Arrival time (0-99)
    int runTime;       // Total runTime (1-10)
    int priority;      // Priority (1-4, 1 is highest)
    int remainingTime; // Remaining runTime
    int startTime;     // Time when the process first gets CPU
    int completionTime;// Time when the process completes
} Process;

// Function declarations
void generateProcesses(Process processes[], int numProcesses);
int compareArrivalTime(const void *a, const void *b);
Process* setupProcesses(int numProcesses);

#endif
