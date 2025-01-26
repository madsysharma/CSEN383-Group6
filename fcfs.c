#include <stdio.h>
#include <stdlib.h>
#include "process_utils.h"
#include "queue_utils.h"

// First-Come First-Served (FCFS) Scheduling
void fcfs(Queue* queue) {
    int currentTime = 0; // Simulation starts at time 0
    float totalTurnaroundTime = 0, totalWaitingTime = 0, totalResponseTime = 0;

    // Create the dynamic timeline
    Timeline* t = createTimeline(100);

    printf("\nFirst-Come First-Served (FCFS) Scheduling:\n");

    while (!isQueueEmpty(queue)) {
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
    int totalProcesses = queue->capacity; // Total number of processes
    printf("\nAverage Turnaround Time: %.2f\n", totalTurnaroundTime / totalProcesses);
    printf("Average Waiting Time: %.2f\n", totalWaitingTime / totalProcesses);
    printf("Average Response Time: %.2f\n", totalResponseTime / totalProcesses);
    printf("Throughput: %.2f processes/unit time\n", (float)totalProcesses / t->size);

    // Free timeline
    freeTimeline(t);
}

int main() {
    int numProcesses;

    printf("Enter the number of processes to simulate: ");
    scanf("%d", &numProcesses);

    // Create a queue with sufficient capacity
    Queue* queue = createQueue(numProcesses);

    // Generate processes and enqueue them
    Process* processes = (Process*)malloc(numProcesses * sizeof(Process));
    generateProcesses(processes, numProcesses);
    for (int i = 0; i < numProcesses; i++) {
        enqueue(queue, processes[i]);
    }
    free(processes);

    // Display generated processes
    printf("\nGenerated Processes:\n");
    printf("Name\tArrival Time\tRuntime\tPriority\n");
    for (int i = 0; i < numProcesses; i++) {
        printf("%c\t%d\t\t%d\t%d\n", queue->processes[i].name, queue->processes[i].arrivalTime,
               queue->processes[i].runtime, queue->processes[i].priority);
    }

    // Run FCFS scheduling
    fcfs(queue);

    // Free the queue
    freeQueue(queue);

    return 0;
}
