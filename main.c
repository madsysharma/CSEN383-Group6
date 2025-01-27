#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <time.h>
#include "process_utils.h"
#include "queue_utils.h"
#include "simulation.h"

void copyProcesses(Process *dest, Process* source, int numProcesses)
{
    for(int i = 0; i < numProcesses; i++)
    {
        dest[i] = source[i];
    }
}

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
        Process* originalProcesses = (Process*)malloc(numProcesses * sizeof(Process));
        generateProcesses(originalProcesses, numProcesses);

        // Display generated processes
        printf("\nGenerated Processes:\n");
        printf("Name\tArrival Time\tRun Time\tPriority\n");
        for (int j = 0; j < numProcesses; j++) 
        {
            printf("%c\t%d\t\t%d\t\t%d\n", originalProcesses[j].name, originalProcesses[j].arrivalTime, originalProcesses[j].runtime, originalProcesses[j].priority);
        }
        float avgTurnaroundTime = 0, avgWaitingTime = 0, avgResponseTime = 0, throughput = 0;
        // Run First Come First Serve scheduling
        Process* fcfsProcesses = (Process*)malloc(numProcesses * sizeof(Process));
        copyProcesses(fcfsProcesses, originalProcesses, numProcesses);
        fcfs(fcfsProcesses, numProcesses, &avgTurnaroundTime, &avgWaitingTime, &avgResponseTime, &throughput);
        totalAvgTurnaroundTime[0] += avgTurnaroundTime;
        totalAvgWaitingTime[0] += avgWaitingTime;
        totalAvgResponseTime[0] += avgResponseTime;
        totalThroughput[0] += throughput;
        free(fcfsProcesses);
        printf("========================================================\n\n");
        // Run Shortest Job First scheduling
        avgTurnaroundTime = 0, avgWaitingTime = 0, avgResponseTime = 0, throughput = 0;
        Process* sjfProcesses = (Process*)malloc(numProcesses * sizeof(Process));
        copyProcesses(sjfProcesses, originalProcesses, numProcesses);
        sjf(sjfProcesses, numProcesses, &avgTurnaroundTime, &avgWaitingTime, &avgResponseTime, &throughput);
        totalAvgTurnaroundTime[1] += avgTurnaroundTime;
        totalAvgWaitingTime[1] += avgWaitingTime;
        totalAvgResponseTime[1] += avgResponseTime;
        totalThroughput[1] += throughput;
        free(sjfProcesses);
        printf("========================================================\n\n");
        // Run Shortest Remaining Time First scheduling
        avgTurnaroundTime = 0, avgWaitingTime = 0, avgResponseTime = 0, throughput = 0;
        Process* srtfProcesses = (Process*)malloc(numProcesses * sizeof(Process));
        copyProcesses(srtfProcesses, originalProcesses, numProcesses);
        srtf(srtfProcesses, numProcesses, &avgTurnaroundTime, &avgWaitingTime, &avgResponseTime, &throughput);
        totalAvgTurnaroundTime[2] += avgTurnaroundTime;
        totalAvgWaitingTime[2] += avgWaitingTime;
        totalAvgResponseTime[2] += avgResponseTime;
        totalThroughput[2] += throughput;
        free(srtfProcesses);
        printf("========================================================\n\n");
        // Run Round Robin scheduling
        avgTurnaroundTime = 0, avgWaitingTime = 0, avgResponseTime = 0, throughput = 0;
        Process* rrProcesses = (Process*)malloc(numProcesses * sizeof(Process));
        copyProcesses(rrProcesses, originalProcesses, numProcesses);
        roundRobin(rrProcesses, numProcesses, &avgTurnaroundTime, &avgWaitingTime, &avgResponseTime, &throughput);
        totalAvgTurnaroundTime[3] += avgTurnaroundTime;
        totalAvgWaitingTime[3] += avgWaitingTime;
        totalAvgResponseTime[3] += avgResponseTime;
        totalThroughput[3] += throughput;
        free(rrProcesses);
        printf("========================================================\n\n");
        // Run Highest Priority First scheduling in non-preemptive mode
        avgTurnaroundTime = 0, avgWaitingTime = 0, avgResponseTime = 0, throughput = 0;
        Process* hpfNPProcesses = (Process*)malloc(numProcesses * sizeof(Process));
        copyProcesses(hpfNPProcesses, originalProcesses, numProcesses);
        simulateHPFNonPreemptive(i+1, hpfNPProcesses, numProcesses, &avgTurnaroundTime, &avgWaitingTime, &avgResponseTime, &throughput);
        totalAvgTurnaroundTime[4] += avgTurnaroundTime;
        totalAvgWaitingTime[4] += avgWaitingTime;
        totalAvgResponseTime[4] += avgResponseTime;
        totalThroughput[4] += throughput;
        free(hpfNPProcesses);
        printf("========================================================\n\n");
        // Run Highest Priority First scheduling in preemptive mode
        avgTurnaroundTime = 0, avgWaitingTime = 0, avgResponseTime = 0, throughput = 0;
        Process* hpfPreProcesses = (Process*)malloc(numProcesses * sizeof(Process));
        copyProcesses(hpfPreProcesses, originalProcesses, numProcesses);
        simulateHPFPreemptive(i+1, hpfPreProcesses, numProcesses, &avgTurnaroundTime, &avgWaitingTime, &avgResponseTime, &throughput);
        totalAvgTurnaroundTime[5] += avgTurnaroundTime;
        totalAvgWaitingTime[5] += avgWaitingTime;
        totalAvgResponseTime[5] += avgResponseTime;
        totalThroughput[5] += throughput;
        free(hpfPreProcesses);
        printf("========================================================\n\n");
        // Free allocated memory
        free(originalProcesses);
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