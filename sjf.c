#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include "process_utils.h"

#define TOTAL_QUANTA 101

void sjf(Process* processes, int numProcesses) {
    int currentTime = 0;
    float totalTurnaroundTime = 0, totalWaitingTime = 0, totalResponseTime = 0;
    int completed = 0;
    int* isCompleted = (int*)calloc(numProcesses, sizeof(int));
    char timeChart[TOTAL_QUANTA + 1];
    memset(timeChart, '-', TOTAL_QUANTA);
    timeChart[TOTAL_QUANTA] = '\0';

    printf("\nShortest Job First (Non-Preemptive) Scheduling:\n");

    while (completed < numProcesses) {
        int minIndex = -1;
        int minRuntime = INT_MAX;

        
        for (int i = 0; i < numProcesses; i++) {
            if (!isCompleted[i] && processes[i].arrivalTime <= currentTime) {
                if (minIndex == -1 || 
                    processes[i].runtime < processes[minIndex].runtime || 
                    (processes[i].runtime == processes[minIndex].runtime && 
                     processes[i].arrivalTime < processes[minIndex].arrivalTime)) {
                    minIndex = i;
                }
            }
        }

        if (minIndex != -1) {
           
            processes[minIndex].startTime = currentTime;
            processes[minIndex].completionTime = currentTime + processes[minIndex].runtime;

            
            for (int t = currentTime; t < processes[minIndex].completionTime && t < TOTAL_QUANTA; t++) {
                timeChart[t] = processes[minIndex].name;
            }

            int turnaroundTime = processes[minIndex].completionTime - processes[minIndex].arrivalTime;
            int waitingTime = turnaroundTime - processes[minIndex].runtime;
            int responseTime = processes[minIndex].startTime - processes[minIndex].arrivalTime;

            totalTurnaroundTime += turnaroundTime;
            totalWaitingTime += waitingTime;
            totalResponseTime += responseTime;

            printf("Process %c: Arrival Time = %d, Runtime = %d, Turnaround Time = %d, Waiting Time = %d, Response Time = %d\n",
                   processes[minIndex].name, 
                   processes[minIndex].arrivalTime, 
                   processes[minIndex].runtime,
                   turnaroundTime, 
                   waitingTime, 
                   responseTime);

            currentTime = processes[minIndex].completionTime;
            isCompleted[minIndex] = 1;
            completed++;
        } else {
            
            currentTime++;
        }
    }

   
    printf("\nAverage Turnaround Time: %.2f\n", totalTurnaroundTime / numProcesses);
    printf("Average Waiting Time: %.2f\n", totalWaitingTime / numProcesses);
    printf("Average Response Time: %.2f\n", totalResponseTime / numProcesses);

    printf("\nTime Chart (total %d quanta): %s\n", TOTAL_QUANTA, timeChart);

    float throughput = (float)completed / TOTAL_QUANTA;
    printf("Throughput: %.2f processes/unit time\n", throughput);

    free(isCompleted);
}

int main() {
    int numProcesses;

   
    printf("Enter the number of processes to simulate: ");
    scanf("%d", &numProcesses);

    if (numProcesses <= 0) {
        printf("Number of processes must be greater than zero.\n");
        return 1;
    }

    
    srand(time(NULL));

    
    Process* processes = (Process*)malloc(numProcesses * sizeof(Process));
    generateProcesses(processes, numProcesses);

   
    printf("\nGenerated Processes:\n");
    printf("Name\tArrival Time\tRuntime\tPriority\n");
    for (int i = 0; i < numProcesses; i++) {
        printf("%c\t%d\t\t%d\t\t%d\n", 
               processes[i].name, 
               processes[i].arrivalTime, 
               processes[i].runtime, 
               processes[i].priority);
    }

    sjf(processes, numProcesses);

    
    free(processes);

    return 0;
}