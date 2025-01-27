#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <time.h>
#include "process_utils.h"
#include "queue_utils.h"

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
void srtf(Process processes[], int numProcesses)
{
	int currTime = 0;
	int completedProcesses = 0;
	float totalTurnaroundTime = 0, totalWaitingTime = 0, totalResponseTime = 0;
    Timeline* t = createTimeline(100);
    Queue* readyQueue = createQueue(numProcesses); // Creating a process queue for all the ready processes
    printf("\nShortest Remaining Time First Scheduling:\n");

    while(completedProcesses < numProcesses)
    {
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
    		// In this case, no process is ready and the CPU is idle - we should ensure that it's not idle for more than 2 quanta
    		int nextArrivalTime = INT_MAX;
            for (int i = 0; i < numProcesses; i++)
            {
                if (processes[i].remainingTime > 0 && processes[i].arrivalTime > currTime)
                {
                    nextArrivalTime = (processes[i].arrivalTime < nextArrivalTime) ? processes[i].arrivalTime : nextArrivalTime;
                }
            }

            int idleTime = (nextArrivalTime == INT_MAX) ? 1 : nextArrivalTime - currTime;
            idleTime = (idleTime > 2) ? 2 : idleTime;  // Keep the maximum idle time to 2 quanta
            updateTimeline(t, currTime, idleTime, '-');
            currTime += idleTime;
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
            	        printf("\nProcess %c: Arrival Time = %d, Runtime = %d, Turnaround Time = %d, Waiting Time = %d, Response Time = %d\n", srtfProcess->name, srtfProcess->arrivalTime, srtfProcess->runtime, tat, wt, rt);

    			//Dequeue the completed process
    			removeFromQueue(readyQueue, srtfProcess);
    		}
    	}
    }

    // Printing the timeline
    printTimeline(t);

    // Printing the calculated averages for turnaround time, response time and waiting time
    printf("\nAverage Turnaround Time: %.2f\n", totalTurnaroundTime / numProcesses);
    printf("\nAverage Waiting Time: %.2f\n", totalWaitingTime / numProcesses);
    printf("\nAverage Response Time: %.2f\n", totalResponseTime / numProcesses);
    printf("\nThroughput: %.2f processes/unit time\n", (float)numProcesses / t->size);

    // Finally, memory has to be freed
    freeTimeline(t);
    freeQueue(readyQueue);
}

int main() 
{
    int numProcesses;
    srand(time(NULL));

    // Input: Number of processes
    printf("Enter the number of processes to simulate: ");
    scanf("%d", &numProcesses);

    int runs = 5;
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

        // Run Shortest Remaining Time First scheduling
        srtf(processes, numProcesses);

        // Free allocated memory
        free(processes);
        printf("\n");
    }
    return 0;
}
