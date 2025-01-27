#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process_utils.h"
#include "queue_utils.h"

#define NUM_PROCESSES 25
#define NUM_RUNS 5
#define MAX_TIME 9999

static int waitingInLevel[1000]; // Tracks waiting time at the current level

void simulateHPFPreemptive(int runIndex);
void runPreemptive(Process *processes, int numProcesses);
void printRunStatistics(Process *processes, int numProcesses, int totalRunTime, Timeline *timeline);

int main()
{
    for (int run = 1; run <= NUM_RUNS; run++)
    {
        simulateHPFPreemptive(run);
        printf("========================================================\n\n");
    }
    return 0;
}

// Runs one simulation of HPF Preemptive for a single run
void simulateHPFPreemptive(int runIndex)
{
    printf("=== HPF Preemptive Run #%d ===\n", runIndex);

    Process *processes = (Process *)malloc(NUM_PROCESSES * sizeof(Process));
    generateProcesses(processes, NUM_PROCESSES);
    runPreemptive(processes, NUM_PROCESSES);
    free(processes);
}

// Preemptive approach: 1-quantum Round Robin within the highest non-empty queue
void runPreemptive(Process *processes, int numProcesses)
{
    Queue *queues[4];
    for (int i = 0; i < 4; i++)
    {
        queues[i] = createQueue(numProcesses);
    }
    Timeline *timeline = createTimeline(200);

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

    int finished = 0;
    int currentTime = 0;

    // Keep going until all active processes finish or time is too large
    while (finished < totalActive && currentTime < MAX_TIME)
    {
        // Enqueue newly arrived processes
        for (int i = 0; i < numProcesses; i++)
        {
            if (processes[i].arrivalTime == currentTime && processes[i].arrivalTime <= 99)
            {
                enqueue(queues[processes[i].priority - 1], processes[i]);
            }
        }

        // Choose the highest non-empty queue
        int chosenQueue = -1;
        for (int q = 0; q < 4; q++)
        {
            if (!isQueueEmpty(queues[q]))
            {
                chosenQueue = q;
                break;
            }
        }

        // If no ready process, CPU is idle for this quantum
        if (chosenQueue == -1)
        {
            updateTimeline(timeline, currentTime, 1, '-');
        }
        else
        {
            Process p = dequeue(queues[chosenQueue]);
            int realIdx = -1;
            for (int i = 0; i < numProcesses; i++)
            {
                if (p.name == processes[i].name &&
                    p.arrivalTime == processes[i].arrivalTime &&
                    p.runtime == processes[i].runtime)
                {
                    realIdx = i;
                    break;
                }
            }
            if (processes[realIdx].startTime == -1)
            {
                processes[realIdx].startTime = currentTime;
            }

            // Run for 1 quantum
            updateTimeline(timeline, currentTime, 1, p.name);
            processes[realIdx].remainingTime--;
            if (processes[realIdx].remainingTime <= 0)
            {
                processes[realIdx].completionTime = currentTime + 1;
                finished++;
            }
            else
            {
                // If not finished, requeue at the same priority level
                enqueue(queues[chosenQueue], processes[realIdx]);
            }
        }

        // Aging: increment wait time for processes still in queues
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
                if (waitingInLevel[realIdx] >= 5)
                {
                    if (q > 0)
                    {
                        temp.priority--;
                        enqueue(queues[q - 1], temp);
                        waitingInLevel[realIdx] = 0;
                    }
                    else
                    {
                        // Already in top queue
                        enqueue(queues[q], temp);
                    }
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

// Prints details of each process plus overall scheduling stats
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
