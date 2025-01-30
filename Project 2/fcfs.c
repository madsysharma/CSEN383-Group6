#include <stdio.h>
#include <stdlib.h>
#include "process_utils.h"
#include "queue_utils.h"
#include "simulation.h"

// First-Come First-Served (FCFS) Scheduling
void fcfs(Process processes[], int numProcesses, float* avgTurnaroundTime, float* avgWaitingTime, float* avgResponseTime, float* throughput) {
    int currentTime = 0; // Simulation starts at time 0
    int completedProcesses = 0; // Tracks the number of completed processes
    float totalTurnaroundTime = 0, totalWaitingTime = 0, totalResponseTime = 0; // Metrics for FCFS scheduling
    Queue* queue = createQueue(numProcesses); // Initializing process queue
    for (int i = 0; i < numProcesses; i++) 
    {
        enqueue(queue, processes[i]);
    }
    // Create the dynamic timeline
    Timeline* t = createTimeline(100);

    printf("\nFirst-Come First-Served (FCFS) Scheduling:\n");

    while (!isQueueEmpty(queue)) {
        // Check the current time to ensure processes don't start at or beyond the 100th quantum
        if (currentTime >= 100)
        {
            printf("Stopping simulation since a process cannot be executed at or beyond the 100th quantum.\n");
            break;
        }
        Process process = dequeue(queue);

        // Handle idle CPU time
        if (currentTime < process.arrivalTime) {
            updateTimeline(t, currentTime, process.arrivalTime - currentTime, '-');
            currentTime = process.arrivalTime;
        }

        // Execute the process
        process.startTime = currentTime;
        updateTimeline(t, currentTime, process.runtime, process.name);

        // Update metrics
        currentTime += process.runtime;
        process.completionTime = currentTime;

        // Increment the number of completed processes
        completedProcesses += 1;

        int turnaroundTime = process.completionTime - process.arrivalTime;
        int waitingTime = turnaroundTime - process.runtime;
        int responseTime = process.startTime - process.arrivalTime;

        totalTurnaroundTime += turnaroundTime;
        totalWaitingTime += waitingTime;
        totalResponseTime += responseTime;

        // Print individual process metrics
        printf("Process %c: Arrival Time = %d, Runtime = %d, Turnaround Time = %d, Waiting Time = %d, Response Time = %d\n",
               process.name, process.arrivalTime, process.runtime, turnaroundTime, waitingTime, responseTime);
    }

    // Print the timeline
    printTimeline(t);

    // Print averages
    *avgTurnaroundTime = totalTurnaroundTime / completedProcesses;
    *avgWaitingTime = totalWaitingTime / completedProcesses;
    *avgResponseTime = totalResponseTime / completedProcesses;
    *throughput = (float)completedProcesses / t->size;
    
    printf("\nAverage Turnaround Time: %.2f\n", *avgTurnaroundTime);
    printf("Average Waiting Time: %.2f\n", *avgWaitingTime);
    printf("Average Response Time: %.2f\n", *avgResponseTime);
    printf("Throughput: %.2f processes/unit time\n", *throughput);

    // Free timeline
    freeTimeline(t);
    freeQueue(queue);
}