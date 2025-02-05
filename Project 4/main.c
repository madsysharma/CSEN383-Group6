#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "helper.h"

// Making a copy of the array of processes
void copyProcesses(Process *dest, Process *source, int numProcesses)
{
	for (int i = 0; i < numProcesses; i++)
	{
		dest[i].id = source[i].id;
		dest[i].arrival_time = source[i].arrival_time;
		dest[i].service_time = source[i].service_time;
		dest[i].num_pages = source[i].num_pages;
		dest[i].starting_page_num = source[i].starting_page_num;
	}
}

int main(int argc, char *argv[])
{
	srand(time(NULL));
	int runs = 5;
	int total_swaps[5] = {0, 0, 0, 0, 0};
	float hit_ratios[5] = {0.0, 0.0, 0.0, 0.0, 0.0};

	for (int i = 0; i < runs; i++)
	{
		printf("RUN #%d\n", i + 1);
		printf("Total processes:%d\n", TOTAL_PROCESSES);
		Process *processes = (Process *)malloc(TOTAL_PROCESSES * sizeof(Process));
		generateProcesses(processes, TOTAL_PROCESSES);
		PageList *plist = (PageList *)malloc(sizeof(PageList));
		initPageList(plist);
		int run_swaps = 0;
		float run_ratio = 0.0;
		// Display generated processes
		printf("\nGenerated Processes:\n");
		printf("ID\tArrival Time\tService Time\tSize in Pages\n");
		for (int j = 0; j < TOTAL_PROCESSES; j++)
		{
			printf("%d\t%d\t\t%d\t\t%d\n", processes[j].id, processes[j].arrival_time, processes[j].service_time, processes[j].num_pages);
		}
		/* Paging algorithm calls and stats calculation done here. Copy original process array into new process array and use that for the paging algorithm.
		Madhuri - LRU
		Vaibhav - MFU
		Yash - LFU
		Nachiket - FIFO
		Archana - Random Pick
		*/
		Process *lruProcesses = (Process *)malloc(TOTAL_PROCESSES * sizeof(Process));
		copyProcesses(lruProcesses, processes, TOTAL_PROCESSES);
		lruSimulation(lruProcesses, TOTAL_PROCESSES, plist, &run_swaps, &run_ratio);
		total_swaps[i] = run_swaps;
		hit_ratios[i] = run_ratio;
		free(lruProcesses);
		free(processes);
		freePageList(plist);
		printf("\n");
	}
	/*
	Averages across 5 runs gets printed here
	*/
	int avg_swaps = 0;
	float avg_hit_ratio = 0.0;
	for (int k = 0; k < 5; k++)
	{
		avg_swaps += total_swaps[k];
		avg_hit_ratio += hit_ratios[k];
	}
	printf("Across all 5 runs: average number of successful swaps=%.2f, average hit ratio=%.2f\n", (float)avg_swaps / 5, (float)avg_hit_ratio / 5);
	return 0;
}