#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <time.h>
#include "process_utils.h"
#include "queue_utils.h"
#include "simulation.h"

// This function returns the process in the ready queue with the shortest remaining time to complete execution
Process* getSRTFProcess(Queue* q)
{
	Process* srtfProcess = NULL;
	int srtfValue = INT_MAX;
	for(int i = 0; i < q->size ; i++)
	{
		int idx = (q->front + i) % (q->capacity);
		Process* curr = &q->processes[idx];
		if(curr->remainingTime > 0 && curr->remainingTime < srtfValue)
		{
			srtfProcess = curr;
			srtfValue = curr->remainingTime;
		}
	}
	return srtfProcess;
}

// This function dequeues the completed process without affecting the rest of the ready queue
void removeFromQueue(Queue* q, Process* p)
{
	Queue* temp = createQueue(q->capacity);
	while(!isQueueEmpty(q))
	{
		Process removed = dequeue(q);
		if(removed.name != p->name)
		{
			enqueue(temp, removed);
		}
	}
	while(!isQueueEmpty(temp))
	{
		enqueue(q,dequeue(temp));
	}
	freeQueue(temp);
}

// This function implements the Shortest Remaining Time First scheduling algorithm
void srtf(Process processes[], int numProcesses, float* avgTurnaroundTime, float* avgWaitingTime, float* avgResponseTime, float* throughput)
{
	int currTime = 0;
	int completedProcesses = 0;
	float totalTurnaroundTime = 0, totalWaitingTime = 0, totalResponseTime = 0;
    Timeline* t = createTimeline(100);
    Queue* readyQueue = createQueue(numProcesses); // Creating a process queue for all the ready processes
    printf("\nShortest Remaining Time First Scheduling:\n");

    while(completedProcesses < numProcesses)
    {
    	// Check the current time to ensure processes don't start at or beyond the 100th quantum
    	if (currTime >= 100)
    	{
    		printf("Stopping simulation since a process cannot be executed at or beyond the 100th quantum.\n");
    		break;
    	}
    	// Enqueue the newly-arrived processes
    	for (int i = 0; i < numProcesses; i++)
    	{
    		if(processes[i].arrivalTime == currTime && processes[i].remainingTime > 0)
    		{
    			enqueue(readyQueue, processes[i]);
    		}

    	}

    	//Invoke function to find process with the shortest remaining time
    	Process* srtfProcess = getSRTFProcess(readyQueue);

    	if(srtfProcess == NULL)
    	{
    		// In this case, no process is ready and the CPU is idle
    		updateTimeline(t, currTime, 1, '-');
            currTime += 1;
    	}
    	else
    	{
    		//SRTF process found - start executing it
    		if(srtfProcess->startTime == -1)
    		{
    			srtfProcess->startTime = currTime; // Mark the process as started - this takes place just once
    		}

    		// Run the process for one quantum of time
    		updateTimeline(t, currTime, 1, srtfProcess->name);
    		srtfProcess->remainingTime -= 1;
    		currTime += 1;

    		// Check if the process is complete
    		if(srtfProcess->remainingTime == 0)
    		{
    			completedProcesses += 1;
    			srtfProcess->completionTime = currTime;

    			// Calculate turnaround time, response time and waiting time
    			int tat = srtfProcess->completionTime - srtfProcess->arrivalTime;
    			int rt = srtfProcess->startTime - srtfProcess->arrivalTime;
    			int wt = tat - srtfProcess->runtime;

    			totalTurnaroundTime += tat;
    			totalResponseTime += rt;
    			totalWaitingTime += wt;

    			// Print the individual metrics
            	printf("Process %c: Arrival Time = %d, Runtime = %d, Turnaround Time = %d, Waiting Time = %d, Response Time = %d\n", srtfProcess->name, srtfProcess->arrivalTime, srtfProcess->runtime, tat, wt, rt);

    			//Dequeue the completed process
    			removeFromQueue(readyQueue, srtfProcess);
    		}
    	}
    }

    // Printing the timeline
    printTimeline(t);

    *avgTurnaroundTime = totalTurnaroundTime / completedProcesses;
    *avgWaitingTime = totalWaitingTime / completedProcesses;
    *avgResponseTime = totalResponseTime / completedProcesses;
    *throughput = (float)completedProcesses / t->size;
    // Printing the calculated averages for turnaround time, response time and waiting time
    printf("\nAverage Turnaround Time: %.2f\n", *avgTurnaroundTime);
    printf("\nAverage Waiting Time: %.2f\n", *avgWaitingTime);
    printf("\nAverage Response Time: %.2f\n", *avgResponseTime);
    printf("\nThroughput: %.2f processes/unit time\n", *throughput);

    // Finally, memory has to be freed
    freeTimeline(t);
    freeQueue(readyQueue);
}
