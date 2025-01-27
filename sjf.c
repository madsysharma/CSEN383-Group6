#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "process_utils.h"
#include "queue_utils.h"


void sjf(Process* processes, int numProcesses, float* avgTurnaroundTime, float* avgWaitingTime, float* avgResponseTime, float* throughput) {
    int currentTime = 0;
    int completedProcesses = 0;
    float totalTurnaroundTime = 0, totalWaitingTime = 0, totalResponseTime = 0;

    // Initialize timeline with sufficient space for all quanta
    Timeline* t = createTimeline(200); // Arbitrarily large size for dynamic tracking
    
    printf("\nShortest Job First (Non-Preemptive) Scheduling:\n");

    // Create a flag to track completed processes
    int* isCompleted = (int*)calloc(numProcesses, sizeof(int));

    while (completedProcesses < numProcesses) {
        // Find the shortest job that has arrived and is not yet completed
        int shortestJobIndex = -1;
        int shortestRuntime = INT_MAX;

        for (int i = 0; i < numProcesses; i++) {
            if (!isCompleted[i] && processes[i].arrivalTime <= currentTime) 
            {
                if (processes[i].runtime < shortestRuntime || (processes[i].runtime == shortestRuntime && processes[i].arrivalTime < processes[shortestJobIndex].arrivalTime))
                {
                    // This is to ensure that the shortest runtime is prioritized, and also to break ties by arrival time
                    shortestRuntime = processes[i].runtime;
                    shortestJobIndex = i;
                }
            }
        }

        if (shortestJobIndex != -1) 
        {
            // Process the selected job
            Process* shortestJob = &processes[shortestJobIndex];
            shortestJob->startTime = currentTime;
            shortestJob->completionTime = currentTime + shortestJob->runtime;

            // Update the timeline with the process execution
            updateTimeline(t, currentTime, shortestJob->runtime, shortestJob->name);

            // Calculate metrics for the process
            int turnaroundTime = shortestJob->completionTime - shortestJob->arrivalTime;
            int waitingTime = turnaroundTime - shortestJob->runtime;
            int responseTime = shortestJob->startTime - shortestJob->arrivalTime;

            totalTurnaroundTime += turnaroundTime;
            totalWaitingTime += waitingTime;
            totalResponseTime += responseTime;

            // Mark the process as completed
            isCompleted[shortestJobIndex] = 1;
            completedProcesses += 1;

            // Print process details
            printf("Process %c: Arrival Time = %d, Runtime = %d, Turnaround Time = %d, Waiting Time = %d, Response Time = %d\n",
                   shortestJob->name, shortestJob->arrivalTime, shortestJob->runtime,
                   turnaroundTime, waitingTime, responseTime);

            // Move time forward
            currentTime = shortestJob->completionTime;
        } 
        else 
        {
            // If no process is ready, mark CPU as idle, look ahead for the next arrival time
            int arrivesNext = INT_MAX;
            for (int i = 0; i < numProcesses; i++)
            {
                if (!isCompleted[i] && processes[i].arrivalTime > currentTime) 
                {
                    arrivesNext = (processes[i].arrivalTime < arrivesNext) ? processes[i].arrivalTime : arrivesNext;
                }
            }

            // Update timeline to account for idle time
            updateTimeline(t, currentTime, arrivesNext - currentTime, '-');
            currentTime = arrivesNext;
        }
    }

    // Print the timeline
    printTimeline(t);

    // Calculate and print averages
    *avgTurnaroundTime = totalTurnaroundTime / numProcesses;
    *avgWaitingTime = totalWaitingTime / numProcesses;
    *avgResponseTime = totalResponseTime / numProcesses;
    *throughput = (float)numProcesses / t->size;

    printf("\nAverage Turnaround Time: %.2f\n", *avgTurnaroundTime);
    printf("Average Waiting Time: %.2f\n", *avgWaitingTime);
    printf("Average Response Time: %.2f\n", *avgResponseTime);
    printf("Throughput: %.2f processes/unit time\n", *throughput);

    // Free allocated memory
    freeTimeline(t);
    free(isCompleted);
}
