#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <time.h>
#include "process_utils.h"
#include "queue_utils.h"
#include "simulation.h"

int main(int argc, char **argv)
{
	int numProcesses;
    srand(time(NULL));

    // Input: Number of processes
    printf("Enter the number of processes to simulate: ");
    scanf("%d", &numProcesses);

    int runs = 5;
    // Arrays to store aggregated averages
    float totalAvgTurnaroundTime[6], totalAvgWaitingTime[6], totalAvgResponseTime[6], totalThroughput[6];
    for (int i = 0; i < 6; i++)
    {
    	totalAvgTurnaroundTime[i] = 0;
    	totalAvgWaitingTime[i] = 0;
    	totalAvgResponseTime[i] = 0;
    	totalThroughput[i] = 0;
    }
    for (int i = 0; i < runs; i++)
    {
        printf("\nRUN %d:\n", i+1);
        // Create and setup processes
        Process* processes = (Process*)malloc(numProcesses * sizeof(Process));
        generateProcesses(processes, numProcesses);

        // Display generated processes
        printf("\nGenerated Processes:\n");
        printf("Name\tArrival Time\tRun Time\tPriority\n");
        for (int j = 0; j < numProcesses; j++) 
        {
            printf("%c\t%d\t\t%d\t\t%d\n", processes[j].name, processes[j].arrivalTime, processes[j].runtime, processes[j].priority);
        }
        float avgTurnaroundTime = 0, avgWaitingTime = 0, avgResponseTime = 0, throughput = 0;
        // Run First Come First Serve scheduling
        fcfs(processes, numProcesses, &avgTurnaroundTime, &avgWaitingTime, &avgResponseTime, &throughput);
        totalAvgTurnaroundTime[0] += avgTurnaroundTime;
        totalAvgWaitingTime[0] += avgWaitingTime;
        totalAvgResponseTime[0] += avgResponseTime;
        totalThroughput[0] += throughput;
        printf("========================================================\n\n");
        // Run Shortest Job First scheduling
        avgTurnaroundTime = 0, avgWaitingTime = 0, avgResponseTime = 0, throughput = 0;
        sjf(processes, numProcesses, &avgTurnaroundTime, &avgWaitingTime, &avgResponseTime, &throughput);
        totalAvgTurnaroundTime[1] += avgTurnaroundTime;
        totalAvgWaitingTime[1] += avgWaitingTime;
        totalAvgResponseTime[1] += avgResponseTime;
        totalThroughput[1] += throughput;
        printf("========================================================\n\n");
        // Run Shortest Remaining Time First scheduling
        avgTurnaroundTime = 0, avgWaitingTime = 0, avgResponseTime = 0, throughput = 0;
        srtf(processes, numProcesses, &avgTurnaroundTime, &avgWaitingTime, &avgResponseTime, &throughput);
        totalAvgTurnaroundTime[2] += avgTurnaroundTime;
        totalAvgWaitingTime[2] += avgWaitingTime;
        totalAvgResponseTime[2] += avgResponseTime;
        totalThroughput[2] += throughput;
        printf("========================================================\n\n");
        // Run Round Robin scheduling
        avgTurnaroundTime = 0, avgWaitingTime = 0, avgResponseTime = 0, throughput = 0;
        roundRobin(processes, numProcesses, &avgTurnaroundTime, &avgWaitingTime, &avgResponseTime, &throughput);
        totalAvgTurnaroundTime[3] += avgTurnaroundTime;
        totalAvgWaitingTime[3] += avgWaitingTime;
        totalAvgResponseTime[3] += avgResponseTime;
        totalThroughput[3] += throughput;
        printf("========================================================\n\n");
        // Run Highest Priority First scheduling in non-preemptive mode
        avgTurnaroundTime = 0, avgWaitingTime = 0, avgResponseTime = 0, throughput = 0;
        simulateHPFNonPreemptive(i+1, processes, numProcesses, &avgTurnaroundTime, &avgWaitingTime, &avgResponseTime, &throughput);
        totalAvgTurnaroundTime[4] += avgTurnaroundTime;
        totalAvgWaitingTime[4] += avgWaitingTime;
        totalAvgResponseTime[4] += avgResponseTime;
        totalThroughput[4] += throughput;
        printf("========================================================\n\n");
        // Run Highest Priority First scheduling in preemptive mode
        avgTurnaroundTime = 0, avgWaitingTime = 0, avgResponseTime = 0, throughput = 0;
        simulateHPFPreemptive(i+1, processes, numProcesses, &avgTurnaroundTime, &avgWaitingTime, &avgResponseTime, &throughput);
        totalAvgTurnaroundTime[5] += avgTurnaroundTime;
        totalAvgWaitingTime[5] += avgWaitingTime;
        totalAvgResponseTime[5] += avgResponseTime;
        totalThroughput[5] += throughput;
        printf("========================================================\n\n");
        // Free allocated memory
        free(processes);
        printf("\n");
    }
    // Calculate and display final averages over all runs
    printf("\nFINAL AVERAGES AFTER %d RUNS:\n", runs);
    printf("FCFS:\n");
    printf("Average Turnaround Time: %.2f\n", totalAvgTurnaroundTime[0] / runs);
    printf("Average Waiting Time: %.2f\n", totalAvgWaitingTime[0] / runs);
    printf("Average Response Time: %.2f\n", totalAvgResponseTime[0] / runs);
    printf("Average Throughput: %.2f processes/unit time\n", totalThroughput[0] / runs);
    printf("========================================================\n\n");
    printf("SJF:\n");
    printf("Average Turnaround Time: %.2f\n", totalAvgTurnaroundTime[1] / runs);
    printf("Average Waiting Time: %.2f\n", totalAvgWaitingTime[1] / runs);
    printf("Average Response Time: %.2f\n", totalAvgResponseTime[1] / runs);
    printf("Average Throughput: %.2f processes/unit time\n", totalThroughput[1] / runs);
    printf("========================================================\n\n");
    printf("SRTF:\n");
    printf("Average Turnaround Time: %.2f\n", totalAvgTurnaroundTime[2] / runs);
    printf("Average Waiting Time: %.2f\n", totalAvgWaitingTime[2] / runs);
    printf("Average Response Time: %.2f\n", totalAvgResponseTime[2] / runs);
    printf("Average Throughput: %.2f processes/unit time\n", totalThroughput[2] / runs);
    printf("========================================================\n\n");
    printf("RR:\n");
    printf("Average Turnaround Time: %.2f\n", totalAvgTurnaroundTime[3] / runs);
    printf("Average Waiting Time: %.2f\n", totalAvgWaitingTime[3] / runs);
    printf("Average Response Time: %.2f\n", totalAvgResponseTime[3] / runs);
    printf("Average Throughput: %.2f processes/unit time\n", totalThroughput[3] / runs);
    printf("========================================================\n\n");
    printf("HPF Non-Preemptive:\n");
    printf("Average Turnaround Time: %.2f\n", totalAvgTurnaroundTime[4] / runs);
    printf("Average Waiting Time: %.2f\n", totalAvgWaitingTime[4] / runs);
    printf("Average Response Time: %.2f\n", totalAvgResponseTime[4] / runs);
    printf("Average Throughput: %.2f processes/unit time\n", totalThroughput[4] / runs);
    printf("========================================================\n\n");
    printf("HPF Preemptive:\n");
    printf("Average Turnaround Time: %.2f\n", totalAvgTurnaroundTime[5] / runs);
    printf("Average Waiting Time: %.2f\n", totalAvgWaitingTime[5] / runs);
    printf("Average Response Time: %.2f\n", totalAvgResponseTime[5] / runs);
    printf("Average Throughput: %.2f processes/unit time\n", totalThroughput[5] / runs);
    printf("========================================================\n\n");
    return 0;
}