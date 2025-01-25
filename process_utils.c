#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process_utils.h"

// Generate random processes
void generateProcesses(Process* processes, int numProcesses) {
    char processName = 'A';
    srand(0); // Fixed random seed for debugging consistency

    for (int i = 0; i < numProcesses; i++) {
        processes[i].name = processName++;
        processes[i].arrivalTime = rand() % 100;       // Random arrival time (0–99)
        processes[i].runtime = (rand() % 10) + 1;     // Random runtime (1–10)
        processes[i].priority = (rand() % 4) + 1;     // Random priority (1–4)
        processes[i].startTime = -1;                  // Not yet started
        processes[i].completionTime = 0;             // Not yet completed
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
    if (requiredCapacity > t->capacity) {
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
