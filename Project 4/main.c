#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "utils.h"

// Making a copy of the array of processes
void copyProcesses(Process *dest, Process* source, int numProcesses)
{
    for(int i = 0; i < numProcesses; i++)
    {
        dest[i] = source[i];
    }
}

int main(int argc, char *argv[])
{
	srand(time(NULL));
	int runs = 5;
	for(int i = 0; i < runs; i++)
	{
		Process* processes = (Process*)malloc(TOTAL_PROCESSES * sizeof(Process));
		generateProcesses(processes, TOTAL_PROCESSES);
		PageList* plist;
		initPageList(plist);
		 // Display generated processes
        printf("\nGenerated Processes:\n");
        printf("ID\tArrival Time\tService Time\tSize in Pages\n");
        for (int j = 0; j < TOTAL_PROCESSES; j++) 
        {
            printf("%c\t%d\t\t%d\t\t%d\n", processes[j].id, processes[j].arrival_time, processes[j].service_time, processes[j].num_pages);
        }
        /* Paging algorithm calls and stats calculation done here. Copy original process array into new process array and use that for the paging algorithm.
        Madhuri - LRU
        Vaibhav - MFU
        Yash - LFU
        Nachiket - FIFO
        Archana - Random Pick
        */
        free(processes);
        freePageList(plist);
        printf("\n");
	}
	/*
	Averages across 5 runs gets printed here
	*/
	return 0;
}