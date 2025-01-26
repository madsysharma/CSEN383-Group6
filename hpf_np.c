#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process_utils.h"
#include "queue_utils.h"

#define NUM_PROCESSES 25
#define NUM_RUNS 1
#define MAX_TIME 9999

static int waitingInLevel[1000]; // Track how long a process has waited at its current level

void simulateHPFNonPreemptive(int runIndex);
void runNonPreemptive(Process *processes, int numProcesses);
void printRunStatistics(Process *processes, int numProcesses, int totalRunTime, Timeline *timeline);

int main()
{
    for (int run = 1; run <= NUM_RUNS; run++)
    {
        simulateHPFNonPreemptive(run);
        printf("========================================================\n\n");
    }
    return 0;
}

// Runs one simulation of HPF (non-preemptive) for one "run"
void simulateHPFNonPreemptive(int runIndex)
{
    printf("=== HPF Non-Preemptive Run #%d ===\n", runIndex);

    Process *processes = (Process *)malloc(NUM_PROCESSES * sizeof(Process));
    generateProcesses(processes, NUM_PROCESSES);
    runNonPreemptive(processes, NUM_PROCESSES);
    free(processes);
}

// Non-preemptive approach: once a process starts, it runs to completion.
void runNonPreemptive(Process *processes, int numProcesses)
{
    Queue *queues[4];
    for (int i = 0; i < 4; i++)
    {
        queues[i] = createQueue(numProcesses);
    }
    Timeline *timeline = createTimeline(200);

    // Identify active processes (arrival <= 99)
    int totalActive = 0;
    for (int i = 0; i < numProcesses; i++)
    {
        if (processes[i].arrivalTime <= 99)
        {
            totalActive++;
        }
        processes[i].startTime = -1;
        processes[i].completionTime = 0;
        processes[i].remainingTime = processes[i].runtime;
        waitingInLevel[i] = 0;
    }

    int finished = 0, currentTime = 0;
    Process *running = NULL;
    int runningIndex = -1;

    // Loop until all active processes finish or time exceeds MAX_TIME
    while (finished < totalActive && currentTime < MAX_TIME)
    {
        // Enqueue newly arrived processes (priority-1 as array index)
        for (int i = 0; i < numProcesses; i++)
        {
            if (processes[i].arrivalTime == currentTime && processes[i].arrivalTime <= 99)
            {
                enqueue(queues[processes[i].priority - 1], processes[i]);
            }
        }

        // Pick next process from highest non-empty queue if not running one
        if (!running)
        {
            for (int q = 0; q < 4; q++)
            {
                if (!isQueueEmpty(queues[q]))
                {
                    Process p = dequeue(queues[q]);
                    for (int i = 0; i < numProcesses; i++)
                    {
                        if (processes[i].name == p.name &&
                            processes[i].arrivalTime == p.arrivalTime &&
                            processes[i].runtime == p.runtime)
                        {
                            runningIndex = i;
                            break;
                        }
                    }
                    running = &processes[runningIndex];
                    if (running->startTime == -1)
                    {
                        running->startTime = currentTime;
                    }
                    break;
                }
            }
        }

        // Run the current process for one quantum
        if (running)
        {
            updateTimeline(timeline, currentTime, 1, running->name);
            running->remainingTime--;
            // If done, record completion and reset
            if (running->remainingTime == 0)
            {
                running->completionTime = currentTime + 1;
                running = NULL;
                runningIndex = -1;
                finished++;
            }
        }
        else
        {
            // CPU idle
            updateTimeline(timeline, currentTime, 1, '-');
        }

        // Aging: increment waiting counter for processes in queues
        for (int q = 0; q < 4; q++)
        {
            int size = queues[q]->size;
            int front = queues[q]->front;
            for (int s = 0; s < size; s++)
            {
                int idx = (front + s) % queues[q]->capacity;
                Process *pr = &queues[q]->processes[idx];
                for (int i = 0; i < numProcesses; i++)
                {
                    if (pr->name == processes[i].name &&
                        pr->arrivalTime == processes[i].arrivalTime &&
                        pr->runtime == processes[i].runtime)
                    {
                        waitingInLevel[i]++;
                        break;
                    }
                }
            }
        }

        // Bump any process that has waited >= 5 quanta in the same level
        for (int q = 1; q < 4; q++)
        {
            int size = queues[q]->size;
            while (size--)
            {
                Process temp = dequeue(queues[q]);
                int realIdx = -1;
                for (int i = 0; i < numProcesses; i++)
                {
                    if (temp.name == processes[i].name &&
                        temp.arrivalTime == processes[i].arrivalTime &&
                        temp.runtime == processes[i].runtime)
                    {
                        realIdx = i;
                        break;
                    }
                }
                if (waitingInLevel[realIdx] >= 5 && q > 0)
                {
                    temp.priority--;
                    enqueue(queues[q - 1], temp);
                    waitingInLevel[realIdx] = 0;
                }
                else
                {
                    enqueue(queues[q], temp);
                }
            }
        }

        currentTime++;
    }

    printRunStatistics(processes, numProcesses, timeline->size, timeline);
    for (int i = 0; i < 4; i++)
    {
        freeQueue(queues[i]);
    }
    freeTimeline(timeline);
}

// Prints the per-process and overall statistics, along with the timeline
void printRunStatistics(Process *processes, int numProcesses, int totalRunTime, Timeline *timeline)
{
    printf("\nProcesses (arrived <= 99):\n");
    printf("Name\tArrival\tRuntime\tPriority\tStart\tCompletion\n");
    for (int i = 0; i < numProcesses; i++)
    {
        if (processes[i].arrivalTime <= 99 && processes[i].startTime != -1)
        {
            printf("%c\t%d\t%d\t%d\t\t%d\t%d\n",
                   processes[i].name,
                   processes[i].arrivalTime,
                   processes[i].runtime,
                   processes[i].priority,
                   processes[i].startTime,
                   processes[i].completionTime);
        }
    }

    printTimeline(timeline);

    double sumTurnaround[4] = {0};
    double sumWaiting[4] = {0};
    double sumResponse[4] = {0};
    int countP[4] = {0};

    double sumT = 0, sumW = 0, sumR = 0;
    int totalFinished = 0;

    // Calculate stats per original priority
    for (int i = 0; i < numProcesses; i++)
    {
        if (processes[i].arrivalTime <= 99 && processes[i].startTime != -1)
        {
            int p = processes[i].priority - 1;
            int turnaround = processes[i].completionTime - processes[i].arrivalTime;
            int waiting = turnaround - processes[i].runtime;
            int response = processes[i].startTime - processes[i].arrivalTime;

            sumTurnaround[p] += turnaround;
            sumWaiting[p] += waiting;
            sumResponse[p] += response;
            countP[p]++;

            sumT += turnaround;
            sumW += waiting;
            sumR += response;
            totalFinished++;
        }
    }

    printf("\nPer-priority stats:\n");
    for (int p = 0; p < 4; p++)
    {
        if (countP[p] > 0)
        {
            double avgT = sumTurnaround[p] / countP[p];
            double avgW = sumWaiting[p] / countP[p];
            double avgR = sumResponse[p] / countP[p];
            printf("Priority %d: Throughput=%d / %d\n", p + 1, countP[p], totalRunTime);
            printf("  Avg Turnaround=%.2f, Avg Waiting=%.2f, Avg Response=%.2f\n",
                   avgT, avgW, avgR);
        }
        else
        {
            printf("Priority %d: Throughput=0 (no processes finished)\n", p + 1);
        }
    }

    if (totalFinished > 0)
    {
        double avgT = sumT / totalFinished;
        double avgW = sumW / totalFinished;
        double avgR = sumR / totalFinished;
        double throughput = (double)totalFinished / totalRunTime;
        printf("\nOverall stats:\n");
        printf("Throughput = %.3f processes/quantum\n", throughput);
        printf("Avg Turnaround=%.2f, Avg Waiting=%.2f, Avg Response=%.2f\n",
               avgT, avgW, avgR);
    }
    else
    {
        printf("\nNo processes started.\n");
    }
}
