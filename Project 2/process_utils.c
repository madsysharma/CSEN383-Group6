#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process_utils.h"

// Generate random processes
void generateProcesses(Process* processes, int numProcesses) {
    char processName = 'A';
    // srand(0); // Fixed random seed for debugging consistency
    for (int i = 0; i < numProcesses; i++) {
        processes[i].name = processName;
        if (processName == 'Z')
        {
            processName = 'a' - 1;
        }
        processName++;
        processes[i].arrivalTime = rand() % 100;       // Random arrival time (0–99)
        processes[i].runtime = (rand() % 10) + 1;     // Random runtime (1–10)
        processes[i].priority = (rand() % 4) + 1;     // Random priority (1–4)
        processes[i].startTime = -1;                  // Not yet started
        processes[i].completionTime = 0;             // Not yet completed
        processes[i].remainingTime = processes[i].runtime;

        if (processes[i].runtime <= 0) 
        {
            fprintf(stderr, "Invalid runtime for process %c\n", processes[i].name);
            exit(EXIT_FAILURE);
        }
    }

    // Sort processes by arrival time
    qsort(processes, numProcesses, sizeof(Process), compareByArrivalTime);
}

// Comparison function for qsort (sort by arrival time)
int compareByArrivalTime(const void* a, const void* b) {
    const Process* p1 = (const Process*)a;
    const Process* p2 = (const Process*)b;
    return p1->arrivalTime - p2->arrivalTime;
}

// Create a new timeline
Timeline* createTimeline(int initialCapacity) {
    Timeline* t = (Timeline*)malloc(sizeof(Timeline));
    t->timeline = (char*)calloc(initialCapacity, sizeof(char));
    t->capacity = initialCapacity;
    t->size = 0;
    return t;
}

// Resize the timeline if more capacity is needed
void resizeTimeline(Timeline* t, int requiredCapacity) {
    //printf("Resizing timeline: Current capacity = %d, Required capacity = %d\n", t->capacity, requiredCapacity);
    if (requiredCapacity > t->capacity) {
        if(requiredCapacity <= 0)
        {
            fprintf(stderr, "Invalid timeline capacity requested!\n");
            exit(EXIT_FAILURE);
        }
        t->capacity = requiredCapacity * 2; // Double the capacity
        t->timeline = (char*)realloc(t->timeline, t->capacity * sizeof(char));
        if (!t->timeline) {
            fprintf(stderr, "Error reallocating timeline memory!\n");
            exit(EXIT_FAILURE);
        }
    }
}

// Update the timeline with a process's execution
void updateTimeline(Timeline* t, int startTime, int duration, char processName) {
     if (startTime < 0 || duration <= 0) {
        fprintf(stderr, "Invalid start time or duration in updateTimeline!\n");
        exit(EXIT_FAILURE);
    }
    int endTime = startTime + duration;
    resizeTimeline(t, endTime);

    for (int i = startTime; i < endTime; i++) {
        t->timeline[i] = processName;
    }

    if (endTime > t->size) {
        t->size = endTime;
    }
}

// Print the timeline
void printTimeline(Timeline* t) {
    printf("\nTime Chart (total %d quanta): ", t->size);
    for (int i = 0; i < t->size; i++) {
        printf("%c", t->timeline[i] ? t->timeline[i] : '-');
    }
    printf("\n");
}

// Free timeline memory
void freeTimeline(Timeline* t) {
    free(t->timeline);
    free(t);
}
