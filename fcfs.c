#include <stdio.h>
#include <stdlib.h>
#include "process_utils.h" // Include the header file

// First-Come First-Served (FCFS) Scheduling
void fcfs(Process processes[], int numProcesses) {
    int currentTime = 0; // Keeps track of the current simulation time
    float totalTurnaroundTime = 0, totalWaitingTime = 0, totalResponseTime = 0;
    char timeline[1000] = {0}; // For storing the time chart (up to 1000 time quanta)

    printf("\nFirst-Come First-Served (FCFS) Scheduling:\n");

    for (int i = 0; i < numProcesses; i++) {
        // If the CPU is idle (current time is less than the process arrival time)
        if (currentTime < (int)processes[i].arrivalTime) {
            currentTime = (int)processes[i].arrivalTime;
        }

        // Set the start time for the process
        if (processes[i].startTime == -1) {
            processes[i].startTime = currentTime;
        }

        // Fill the timeline with the process name during its execution
        for (int t = 0; t < processes[i].burstTime; t++) {
            timeline[currentTime + t] = processes[i].name;
        }

        // Update the current time after the process completes execution
        currentTime += processes[i].burstTime;

        // Set the completion time
        processes[i].completionTime = currentTime;

        // Calculate Turnaround Time, Waiting Time, and Response Time
        int turnaroundTime = (int)(processes[i].completionTime - processes[i].arrivalTime + 0.5); // Rounding for precision
        int waitingTime = turnaroundTime - processes[i].burstTime;
        int responseTime = (int)(processes[i].startTime - processes[i].arrivalTime + 0.5); // Rounding for precision

        // Ensure no negative times
        if (waitingTime < 0) waitingTime = 0;
        if (responseTime < 0) responseTime = 0;

        // Update totals for averages
        totalTurnaroundTime += turnaroundTime;
        totalWaitingTime += waitingTime;
        totalResponseTime += responseTime;

        // Print individual process metrics
        printf("Process %c: Arrival Time = %.2f, Burst Time = %d, Turnaround Time = %d, Waiting Time = %d, Response Time = %d\n",
               processes[i].name, processes[i].arrivalTime, processes[i].burstTime, turnaroundTime, waitingTime, responseTime);
    }

    // Print the time chart of execution
    printf("\nTime Chart: ");
    for (int t = 0; t < currentTime; t++) {
        printf("%c", timeline[t]);
    }
    printf("\n");

    // Calculate and print average metrics
    float throughput = (float)numProcesses / (currentTime - (int)processes[0].arrivalTime); // Processes completed per unit time
    printf("\nAverage Turnaround Time: %.2f\n", totalTurnaroundTime / numProcesses);
    printf("Average Waiting Time: %.2f\n", totalWaitingTime / numProcesses);
    printf("Average Response Time: %.2f\n", totalResponseTime / numProcesses);
    printf("Throughput: %.2f processes/unit time\n", throughput);
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
    printf("Name\tArrival Time\tBurst Time\tPriority\n");
    for (int i = 0; i < numProcesses; i++) {
        printf("%c\t%.2f\t\t%d\t\t%d\n", processes[i].name, processes[i].arrivalTime, processes[i].burstTime, processes[i].priority);
    }

    // Run FCFS scheduling
    fcfs(processes, numProcesses);

    // Free allocated memory
    free(processes);

    return 0;
}

