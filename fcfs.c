#include <stdio.h>
#include <stdlib.h>
#include "process_utils.h" // Include the header file

// First-Come First-Served (FCFS) Scheduling
void fcfs(Process processes[], int numProcesses) {
    int currentTime = 0; // Simulation time starts at 0
    float totalTurnaroundTime = 0, totalWaitingTime = 0, totalResponseTime = 0;
    char timeline[100] = {0}; // For storing the time chart for 100 quanta

    printf("\nFirst-Come First-Served (FCFS) Scheduling:\n");

    for (int i = 0; i < numProcesses; i++) {
        // Ignore processes arriving after 99 quanta
        if (processes[i].arrivalTime > 99) {
            continue;
        }

        // If the CPU is idle (current time is less than the process arrival time)
        if (currentTime < processes[i].arrivalTime) {
            currentTime = processes[i].arrivalTime;
        }

        // Set the start time for the process
        if (processes[i].startTime == -1) {
            processes[i].startTime = currentTime;
        }

        // Fill the timeline with the process name during its execution (up to 100 quanta)
        for (int t = 0; t < processes[i].burstTime && currentTime + t < 100; t++) {
            timeline[currentTime + t] = processes[i].name;
        }

        // Update the current time after the process completes or hits 100 quanta
        currentTime += processes[i].burstTime;

        if (currentTime > 100) {
            processes[i].completionTime = 100; // Clamp completion time to 100
            break; // Stop execution after 100 quanta
        } else {
            processes[i].completionTime = currentTime;
        }

        // Calculate Turnaround Time, Waiting Time, and Response Time
        int turnaroundTime = processes[i].completionTime - processes[i].arrivalTime;
        int waitingTime = turnaroundTime - processes[i].burstTime;
        int responseTime = processes[i].startTime - processes[i].arrivalTime;

        // Update totals for averages
        totalTurnaroundTime += turnaroundTime;
        totalWaitingTime += waitingTime;
        totalResponseTime += responseTime;

        // Print individual process metrics
        printf("Process %c: Arrival Time = %d, Burst Time = %d, Turnaround Time = %d, Waiting Time = %d, Response Time = %d\n",
               processes[i].name, (int)processes[i].arrivalTime, processes[i].burstTime, turnaroundTime, waitingTime, responseTime);
    }

    // Print the time chart of execution
    printf("\nTime Chart (first 100 quanta): ");
    for (int t = 0; t < 100; t++) {
        printf("%c", timeline[t] ? timeline[t] : '-'); // Use '-' for idle time
    }
    printf("\n");

    // Calculate and print average metrics
    float throughput = (float)numProcesses / 100; // Processes completed per 100 quanta
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

