#include <stdio.h>
#include <stdlib.h>
#include "process_utils.h"

// Function to generate random processes
void generateProcesses(Process processes[], int numProcesses) {
    srand(0); // Seed the random number generator for reproducibility
    for (int i = 0; i < numProcesses; i++) {
        processes[i].name = 'A' + i;                   // Assign a name (A, B, C, ...)
        processes[i].arrivalTime = rand() % 100;       // Random integer [0, 99]
        processes[i].burstTime = rand() % 10 + 1;      // Random integer [1, 10]
        processes[i].priority = rand() % 4 + 1;        // Random integer [1, 4]
        processes[i].remainingTime = processes[i].burstTime; // Initialize remaining time
        processes[i].startTime = -1;                  // Not yet started
        processes[i].completionTime = 0;              // Not yet completed
    }
}


// Function to sort processes by arrival time
int compareArrivalTime(const void *a, const void *b) {
    Process *p1 = (Process *)a;
    Process *p2 = (Process *)b;
    return (p1->arrivalTime > p2->arrivalTime) - (p1->arrivalTime < p2->arrivalTime);
}

// Function to setup processes (generate and sort by arrival time)
Process* setupProcesses(int numProcesses) {
    Process* processes = (Process*)malloc(numProcesses * sizeof(Process));
    if (!processes) {
        printf("Memory allocation failed!\n");
        exit(1);
    }
    generateProcesses(processes, numProcesses);
    qsort(processes, numProcesses, sizeof(Process), compareArrivalTime); // Sort by arrival time
    return processes;
}

