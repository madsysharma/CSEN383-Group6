#include <stdio.h>
#include <stdlib.h>
#include "process_utils.h"

// Round Robin Scheduling
void rr(Process processes[], int numProcesses) {
    int currentTime = 0;
    int completedProcesses = 0;
    float totalTurnaroundTime = 0, totalWaitingTime = 0, totalResponseTime = 0;
    int timelineSize = 100; // Initial size of the timeline
    char *timeline = (char *)calloc(timelineSize, sizeof(char)); // Dynamically allocate memory for the timeline

    if (!timeline) {
        printf("Memory allocation failed for timeline.\n");
        return;
    } 

    printf("\nRound Robin Scheduling (Time Quantum = 1):\n");

    // Initialize a queue for process indices
    int queue[MAX_PROCESSES], front = 0, rear = 0;

    while (completedProcesses < numProcesses || currentTime < 100) {
        // Grow the timeline dynamically
        if (currentTime >= timelineSize) {
            timelineSize *= 2; // Double the size
            timeline = (char *)realloc(timeline, timelineSize * sizeof(char));
            if (!timeline) {
                printf("Memory reallocation failed.\n");
                return;
            }
            printf("Timeline size is updated to : %d\n", timelineSize);
        }

        // Add newly arrived processes at the current time to the queue
        for (int i = 0; i < numProcesses; i++) {
            if (processes[i].arrivalTime == currentTime) {
                queue[rear++] = i;
            }
        }
        if (front == rear) {
            // CPU idle if no processes are in the queue
            timeline[currentTime++] = '-';
            continue;
        }

        // Dequeue the next process
        int index = queue[front++];

        // If the process hasn't started yet, set its start time
        if (processes[index].startTime == -1) {
            processes[index].startTime = currentTime;
        }

        // Execute the process for 1 quantum
        timeline[currentTime++] = processes[index].name;
        processes[index].remainingTime -= 1;

        // If the process is complete
        if (processes[index].remainingTime == 0) {
            processes[index].completionTime = currentTime;
            completedProcesses++;

            // Calculate metrics for the process
            int turnaroundTime = processes[index].completionTime - processes[index].arrivalTime;
            int waitingTime = turnaroundTime - processes[index].runTime;
            int responseTime = processes[index].startTime - processes[index].arrivalTime;

            totalTurnaroundTime += turnaroundTime;
            totalWaitingTime += waitingTime;
            totalResponseTime += responseTime;

            printf("Process %c: Arrival Time = %d, Run Time = %d, Turnaround Time = %d, Waiting Time = %d, Response Time = %d\n",
                   processes[index].name, processes[index].arrivalTime, processes[index].runTime,
                   turnaroundTime, waitingTime, responseTime);
        } else {
            // Re-enqueue the process if not complete
            queue[rear++] = index;
        }
    }

    // Print the time chart of execution
    printf("\nTime Chart (first 100 quanta or till the last process completes):\n");
    for (int t = 0; t < currentTime; t += 20) {
        // Print the time numbers
        for (int i = t; i < t + 20 && i < currentTime; i++) {
            printf("%3d ", i); // Print time indices (1-based indexing)
        }
        printf("\n");
        // Print the corresponding processes or idle times
        for (int i = t; i < t + 20 && i < currentTime; i++) {
            printf(" %2c ", timeline[i] ? timeline[i] : '-');
        }
        printf("\n\n");
    }


    // Calculate and print average metrics
    printf("\nAverage Turnaround Time: %.2f\n", totalTurnaroundTime / numProcesses);
    printf("Average Waiting Time: %.2f\n", totalWaitingTime / numProcesses);
    printf("Average Response Time: %.2f\n", totalResponseTime / numProcesses);
    printf("Throughput: %.3f processes/unit time\n", (float)completedProcesses / currentTime);

    free(timeline);
}

int main() {
    int numProcesses;

    // Input: Number of processes
    printf("Enter the number of processes to simulate: ");
    scanf("%d", &numProcesses);

    if (numProcesses > MAX_PROCESSES) {
        printf("Number of processes cannot exceed %d.\n", MAX_PROCESSES);
        return 1;
    }

    // Create and setup processes
    Process* processes = setupProcesses(numProcesses);

    // Display generated processes
    printf("\nGenerated Processes:\n");
    printf("Name\tArrival Time\tRun Time\tPriority\n");
    for (int i = 0; i < numProcesses; i++) {
        printf("%c\t%d\t\t%d\t\t%d\n", processes[i].name, processes[i].arrivalTime, processes[i].runTime, processes[i].priority);
    }

    // Run Round Robin scheduling
    rr(processes, numProcesses);

    // Free allocated memory
    free(processes);

    return 0;
}
