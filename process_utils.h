#ifndef PROCESS_UTILS_H
#define PROCESS_UTILS_H

// Process structure
typedef struct Process {
    char name;         // Process name
    int arrivalTime;   // Arrival time
    int runtime;       // Total runtime
    int priority;      // Priority level
    int startTime;     // Time process first gets the CPU
    int completionTime;// Time process finishes execution
} Process;

// Timeline structure
typedef struct {
    char* timeline;    // Dynamically allocated timeline
    int capacity;      // Current capacity of the timeline
    int size;          // Current size of the timeline (used quanta)
} Timeline;

// Process-related utilities
void generateProcesses(Process* processes, int numProcesses);
int compareByArrivalTime(const void* a, const void* b);

// Timeline utilities
Timeline* createTimeline(int initialCapacity);
void resizeTimeline(Timeline* t, int requiredCapacity);
void updateTimeline(Timeline* t, int startTime, int duration, char processName);
void printTimeline(Timeline* t);
void freeTimeline(Timeline* t);

#endif
