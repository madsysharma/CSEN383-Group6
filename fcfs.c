#include <stdio.h>
#include <stdlib.h>
#include "process_utils.h"

// First-Come First-Served (FCFS) Scheduling
void fcfs(Process* processes, int numProcesses) {
    int currentTime = 0;
    float totalTurnaroundTime = 0, totalWaitingTime = 0, totalResponseTime = 0;

    // Create the dynamic timeline
    Timeline* t = createTimeline(100);

    printf("\nFirst-Come First-Served (FCFS) Scheduling:\n");

    for (int i = 0; i < numProcesses; i++) {
        // Handle idle CPU time
        if (currentTime < processes[i].arrivalTime) {
            updateTimeline(t, currentTime, processes[i].arrivalTime - currentTime, '-');
            currentTime = processes[i].arrivalTime;
        }

        // Execute the process
        processes[i].startTime = currentTime;
        updateTimeline(t, currentTime, processes[i].runtime, processes[i].name);

        // Update metrics
        currentTime += processes[i].runtime;
        processes[i].completionTime = currentTime;

        int turnaroundTime = processes[i].completionTime - processes[i].arrivalTime;
        int waitingTime = turnaroundTime - processes[i].runtime;
        int responseTime = processes[i].startTime - processes[i].arrivalTime;

        totalTurnaroundTime += turnaroundTime;
        totalWaitingTime += waitingTime;
        totalResponseTime += responseTime;

        // Print individual metrics
        printf("Process %c: Arrival Time = %d, Runtime = %d, Turnaround Time = %d, Waiting Time = %d, Response Time = %d\n",
               processes[i].name, processes[i].arrivalTime, processes[i].runtime,
               turnaroundTime, waitingTime, responseTime);
    }

    // Print the timeline
    printTimeline(t);

    // Print averages
    printf("\nAverage Turnaround Time: %.2f\n", totalTurnaroundTime / numProcesses);
    printf("Average Waiting Time: %.2f\n", totalWaitingTime / numProcesses);
    printf("Average Response Time: %.2f\n", totalResponseTime / numProcesses);
    printf("Throughput: %.2f processes/unit time\n", (float)numProcesses / t->size);

    // Free timeline
    freeTimeline(t);
}

int main() {
    int numProcesses;

    printf("Enter the number of processes to simulate: ");
    scanf("%d", &numProcesses);

    Process* processes = (Process*)malloc(numProcesses * sizeof(Process));
    generateProcesses(processes, numProcesses);

    printf("\nGenerated Processes:\n");
    printf("Name\tArrival Time\tRuntime\tPriority\n");
    for (int i = 0; i < numProcesses; i++) {
        printf("%c\t%d\t\t%d\t%d\n", processes[i].name, processes[i].arrivalTime,
               processes[i].runtime, processes[i].priority);
    }

    fcfs(processes, numProcesses);

    free(processes);
    return 0;
}
