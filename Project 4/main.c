#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
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
	(void)argc;
	(void)argv;
	srand(time(NULL));
	int runs = 5;
	int lru_total_swaps[5] = {0, 0, 0, 0, 0};
	float lru_hit_ratios[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
	int lfu_total_swaps[5] = {0, 0, 0, 0, 0};
	float lfu_hit_ratios[5] = {0.0, 0.0, 0.0, 0.0, 0.0};

	for (int i = 0; i < runs; i++)
	{
		printf("RUN #%d\n", i + 1);
		printf("Total processes: %d\n", TOTAL_PROCESSES);

		// Generate a set of processes
		Process *processes = (Process *)malloc(TOTAL_PROCESSES * sizeof(Process));
		generateProcesses(processes, TOTAL_PROCESSES);

		// Display generated processes
		printf("\nGenerated Processes:\n");
		printf("ID\tArrival Time\tService Time\tSize in Pages\n");
		for (int j = 0; j < TOTAL_PROCESSES; j++)
		{
			printf("%d\t%d\t\t%d\t\t%d\n", processes[j].id, processes[j].arrival_time,
				   processes[j].service_time, processes[j].num_pages);
		}

		// ---------------- LRU Simulation ----------------
		// Create a copy of the processes array for LRU simulation
		Process *lruProcesses = (Process *)malloc(TOTAL_PROCESSES * sizeof(Process));
		copyProcesses(lruProcesses, processes, TOTAL_PROCESSES);
		// Allocate a new page list for LRU simulation
		PageList *lru_plist = (PageList *)malloc(sizeof(PageList));
		initPageList(lru_plist);
		int run_swaps = 0;
		float run_ratio = 0.0;
		lruSimulation(lruProcesses, TOTAL_PROCESSES, lru_plist, &run_swaps, &run_ratio);
		lru_total_swaps[i] = run_swaps;
		lru_hit_ratios[i] = run_ratio;
		free(lruProcesses);
		freePageList(lru_plist);

		// ---------------- LFU Simulation ----------------
		// Create a copy of the processes array for LFU simulation
		Process *lfuProcesses = (Process *)malloc(TOTAL_PROCESSES * sizeof(Process));
		copyProcesses(lfuProcesses, processes, TOTAL_PROCESSES);
		// Allocate a new page list for LFU simulation
		PageList *lfu_plist = (PageList *)malloc(sizeof(PageList));
		initPageList(lfu_plist);
		int run_swapped_in = 0;
		float run_lfu_ratio = 0.0;
		lfuSimulation(lfuProcesses, TOTAL_PROCESSES, lfu_plist, &run_swapped_in, &run_lfu_ratio);
		lfu_total_swaps[i] = run_swapped_in;
		lfu_hit_ratios[i] = run_lfu_ratio;
		free(lfuProcesses);
		freePageList(lfu_plist);

		free(processes);
		printf("\n");
	}

	// Print LRU simulation averages
	int avg_lru_swaps = 0;
	float avg_lru_hit_ratio = 0.0;
	for (int k = 0; k < runs; k++)
	{
		avg_lru_swaps += lru_total_swaps[k];
		avg_lru_hit_ratio += lru_hit_ratios[k];
	}
	printf("LRU Simulation: Average number of successful swaps = %.2f, average hit ratio = %.2f\n",
		   (float)avg_lru_swaps / runs, avg_lru_hit_ratio / runs);

	// Print LFU simulation averages
	int avg_lfu_swaps = 0;
	float avg_lfu_hit_ratio = 0.0;
	for (int k = 0; k < runs; k++)
	{
		avg_lfu_swaps += lfu_total_swaps[k];
		avg_lfu_hit_ratio += lfu_hit_ratios[k];
	}
	printf("LFU Simulation: Average number of processes swapped-in = %.2f, average hit ratio = %.2f\n",
		   (float)avg_lfu_swaps / runs, avg_lfu_hit_ratio / runs);

	return 0;
}
