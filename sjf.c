#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include "process_utils.h"
#include "queue_utils.h"
#include "simulation.h"  

#define TOTAL_QUANTA 101

void sjf(Process* processes, int numProcesses, float* avgTurnaroundTime, float* avgWaitingTime, float* avgResponseTime, float* throughput) {
    int currentTime = 0;
    float totalTurnaroundTime = 0, totalWaitingTime = 0, totalResponseTime = 0;
    int completed = 0;
    int* isCompleted = (int*)calloc(numProcesses, sizeof(int));
    char timeChart[TOTAL_QUANTA + 1];
    memset(timeChart, '-', TOTAL_QUANTA);
    timeChart[TOTAL_QUANTA] = '\0';

    // Create a queue for ready processes
    Queue* readyQueue = createQueue(numProcesses);

    printf("\nShortest Job First (Non-Preemptive) Scheduling:\n");

    while (completed < numProcesses) {
        // Enqueue processes that have arrived and are not yet completed
        for (int i = 0; i < numProcesses; i++) {
            if (!isCompleted[i] && processes[i].arrivalTime <= currentTime) {
                enqueue(readyQueue, processes[i]);
                isCompleted[i] = 1;  // Mark as enqueued to avoid re-enqueueing
            }
        }

        // Find the process with the shortest runtime in the queue
        int minIndex = -1;
        int minRuntime = INT_MAX;
        for (int i = 0; i < readyQueue->size; i++) {
            int idx = (readyQueue->front + i) % readyQueue->capacity;
            Process* p = &readyQueue->processes[idx];
            if (p->runtime < minRuntime) {
                minRuntime = p->runtime;
                minIndex = idx;
            }
        }

        if (minIndex != -1) {
            // Dequeue the process with the shortest runtime
            Process shortestJob = dequeue(readyQueue);

            // Execute the process
            shortestJob.startTime = currentTime;
            shortestJob.completionTime = currentTime + shortestJob.runtime;

            // Update the timeline
            for (int t = currentTime; t < shortestJob.completionTime && t < TOTAL_QUANTA; t++) {
                timeChart[t] = shortestJob.name;
            }

            // Calculate metrics
            int turnaroundTime = shortestJob.completionTime - shortestJob.arrivalTime;
            int waitingTime = turnaroundTime - shortestJob.runtime;
            int responseTime = shortestJob.startTime - shortestJob.arrivalTime;

            totalTurnaroundTime += turnaroundTime;
            totalWaitingTime += waitingTime;
            totalResponseTime += responseTime;

            printf("Process %c: Arrival Time = %d, Runtime = %d, Turnaround Time = %d, Waiting Time = %d, Response Time = %d\n",
                   shortestJob.name, 
                   shortestJob.arrivalTime, 
                   shortestJob.runtime,
                   turnaroundTime, 
                   waitingTime, 
                   responseTime);

            currentTime = shortestJob.completionTime;
            completed++;
        } else {
            // No process is ready, increment time
            currentTime++;
        }
    }

    // Print averages
    *avgTurnaroundTime = totalTurnaroundTime / numProcesses;
    *avgWaitingTime = totalWaitingTime / numProcesses;
    *avgResponseTime = totalResponseTime / numProcesses;
    *throughput = (float)completed / TOTAL_QUANTA;
    printf("\nAverage Turnaround Time: %.2f\n", *avgTurnaroundTime);
    printf("Average Waiting Time: %.2f\n", *avgWaitingTime);
    printf("Average Response Time: %.2f\n", *avgResponseTime);

    // Print the timeline
    printf("\nTime Chart (total %d quanta): %s\n", TOTAL_QUANTA, timeChart);

    // Calculate throughput
    printf("Throughput: %.2f processes/unit time\n", *throughput);

    // Free allocated memory
    free(isCompleted);
    freeQueue(readyQueue);  // Free the queue
}
