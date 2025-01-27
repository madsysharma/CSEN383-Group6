#include <stdio.h>
#include <stdlib.h>
#include "process_utils.h"
#include "queue_utils.h"
#include "simulation.h"

void roundRobin(Process processes[], int numProcesses, float* avgTurnaroundTime, float* avgWaitingTime, float* avgResponseTime, float* throughput) {
    int currTime = 0, completedProcesses = 0;
    float totalTurnaroundTime = 0, totalWaitingTime = 0, totalResponseTime = 0;
    Timeline* t = createTimeline(100);
    Queue* readyQueue = createQueue(numProcesses);

    printf("\nRound Robin Scheduling (Quantum = 1):\n");

    while (completedProcesses < numProcesses) {
        // Add processes arriving at the current time to the ready queue
        for (int i = 0; i < numProcesses; i++) {
            if (processes[i].arrivalTime == currTime && processes[i].remainingTime > 0) {
                enqueue(readyQueue, processes[i]);
            }
        }

        if (!isQueueEmpty(readyQueue)) {
            // Fetch the next process from the ready queue
            Process currProcess = dequeue(readyQueue);

            if (currProcess.startTime == -1) {
                currProcess.startTime = currTime; // Mark the process as started
            }

            // Execute the process for one quantum
            updateTimeline(t, currTime, 1, currProcess.name);

            currTime++;
            currProcess.remainingTime--;

            if (currProcess.remainingTime == 0) {
                // Process has completed
                completedProcesses++;
                currProcess.completionTime = currTime;

                int tat = currProcess.completionTime - currProcess.arrivalTime;
                int rt = currProcess.startTime - currProcess.arrivalTime;
                int wt = tat - currProcess.runtime;

                totalTurnaroundTime += tat;
                totalResponseTime += rt;
                totalWaitingTime += wt;

                // printf("\nProcess %c: Arrival Time = %d, Runtime = %d, Turnaround Time = %d, Waiting Time = %d, Response Time = %d\n",
                //        currProcess.name, currProcess.arrivalTime, currProcess.runtime, tat, wt, rt);
            } else {
                // Process has not completed; re-add it to the ready queue
                enqueue(readyQueue, currProcess);
            }
        } else {
            // CPU is idle
            updateTimeline(t, currTime, 1, '-');
            currTime++;
        }
    }

    printTimeline(t);

    // Calculate averages for this run
    *avgTurnaroundTime = totalTurnaroundTime / numProcesses;
    *avgWaitingTime = totalWaitingTime / numProcesses;
    *avgResponseTime = totalResponseTime / numProcesses;
    *throughput = (float)numProcesses / t->size;

    // Display averages
    printf("\nAverage Turnaround Time: %.2f\n", *avgTurnaroundTime);
    printf("Average Waiting Time: %.2f\n", *avgWaitingTime);
    printf("Average Response Time: %.2f\n", *avgResponseTime);
    printf("Throughput: %.2f processes/unit time\n", *throughput);

    // Free allocated resources
    freeTimeline(t);
    freeQueue(readyQueue);
}