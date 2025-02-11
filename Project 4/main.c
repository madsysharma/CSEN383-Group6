#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "helper.h"

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
    
    int rp_total_swaps[5] = {0, 0, 0, 0, 0};
    float rp_hit_ratios[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
    
    int fifo_total_swaps[5] = {0, 0, 0, 0, 0};
    float fifo_hit_ratios[5] = {0.0, 0.0, 0.0, 0.0, 0.0};

    int mfu_total_swaps[5] = {0, 0, 0, 0, 0};
    float mfu_hit_ratios[5] = {0.0, 0.0, 0.0, 0.0, 0.0};

    for (int i = 0; i < runs; i++)
    {
        printf("RUN #%d\n", i + 1);
        printf("Total processes: %d\n", TOTAL_PROCESSES);

        // Generate a set of processes
        Process *processes = (Process *)malloc(TOTAL_PROCESSES * sizeof(Process));
        generateProcesses(processes, TOTAL_PROCESSES);

        // ---------------- FIFO Simulation ----------------
        Process *fifoProcesses = (Process *)malloc(TOTAL_PROCESSES * sizeof(Process));
        copyProcesses(fifoProcesses, processes, TOTAL_PROCESSES);
        PageList *fifo_plist = (PageList *)malloc(sizeof(PageList));
        initPageList(fifo_plist);
        int fifo_run_swaps = 0;
        float fifo_run_ratio = 0.0;
        fifoSimulation(fifoProcesses, TOTAL_PROCESSES, fifo_plist, &fifo_run_swaps, &fifo_run_ratio);
        fifo_total_swaps[i] = fifo_run_swaps;
        fifo_hit_ratios[i] = fifo_run_ratio;
        free(fifoProcesses);
        freePageList(fifo_plist);

        // ---------------- LRU Simulation ----------------
        Process *lruProcesses = (Process *)malloc(TOTAL_PROCESSES * sizeof(Process));
        copyProcesses(lruProcesses, processes, TOTAL_PROCESSES);
        PageList *lru_plist = (PageList *)malloc(sizeof(PageList));
        initPageList(lru_plist);
        int lru_run_swaps = 0;
        float lru_run_ratio = 0.0;
        lruSimulation(lruProcesses, TOTAL_PROCESSES, lru_plist, &lru_run_swaps, &lru_run_ratio);
        lru_total_swaps[i] = lru_run_swaps;
        lru_hit_ratios[i] = lru_run_ratio;
        free(lruProcesses);
        freePageList(lru_plist);

        // ---------------- LFU Simulation ----------------
        Process *lfuProcesses = (Process *)malloc(TOTAL_PROCESSES * sizeof(Process));
        copyProcesses(lfuProcesses, processes, TOTAL_PROCESSES);
        PageList *lfu_plist = (PageList *)malloc(sizeof(PageList));
        initPageList(lfu_plist);
        int lfu_run_swaps = 0;
        float lfu_run_ratio = 0.0;
        lfuSimulation(lfuProcesses, TOTAL_PROCESSES, lfu_plist, &lfu_run_swaps, &lfu_run_ratio);
        lfu_total_swaps[i] = lfu_run_swaps;
        lfu_hit_ratios[i] = lfu_run_ratio;
        free(lfuProcesses);
        freePageList(lfu_plist);

        // ---------------- Random Pick Simulation ----------------
        Process *rpProcesses = (Process *)malloc(TOTAL_PROCESSES * sizeof(Process));
        copyProcesses(rpProcesses, processes, TOTAL_PROCESSES);
        PageList *rp_plist = (PageList *)malloc(sizeof(PageList));
        initPageList(rp_plist);
        int rp_run_swaps = 0;
        float rp_run_ratio = 0.0;
        randomPickSimulation(rpProcesses, TOTAL_PROCESSES, rp_plist, &rp_run_swaps, &rp_run_ratio);
        rp_total_swaps[i] = rp_run_swaps;
        rp_hit_ratios[i] = rp_run_ratio;
        free(rpProcesses);
        freePageList(rp_plist);

        // ---------------- MFU Simulation ----------------
        Process *mfuProcesses = (Process *)malloc(TOTAL_PROCESSES * sizeof(Process));
        copyProcesses(mfuProcesses, processes, TOTAL_PROCESSES);
        PageList *mfu_plist = (PageList *)malloc(sizeof(PageList));
        initPageList(mfu_plist);
        int mfu_run_swaps = 0;
        float mfu_run_ratio = 0.0;
        mfuSimulation(mfuProcesses, TOTAL_PROCESSES, mfu_plist, &mfu_run_swaps, &mfu_run_ratio);
        mfu_total_swaps[i] = mfu_run_swaps;
        mfu_hit_ratios[i] = mfu_run_ratio;
        free(mfuProcesses);
        freePageList(mfu_plist);

        free(processes);
        printf("\n");
    }

    // Print simulation averages
    printf("After all 5 runs:\n");

    int avg_fifo_swaps = 0;
    float avg_fifo_hit_ratio = 0.0;
    for (int k = 0; k < runs; k++)
    {
        avg_fifo_swaps += fifo_total_swaps[k];
        avg_fifo_hit_ratio += fifo_hit_ratios[k];
    }
    printf("-----------------------------------------------------------------------------------------------\n");
    printf("FIFO Simulation: Average number of successful swaps = %.2f, average hit ratio = %.5f\n",
           (float)avg_fifo_swaps / runs, avg_fifo_hit_ratio / runs);

    int avg_lru_swaps = 0;
    float avg_lru_hit_ratio = 0.0;
    for (int k = 0; k < runs; k++)
    {
        avg_lru_swaps += lru_total_swaps[k];
        avg_lru_hit_ratio += lru_hit_ratios[k];
    }
    printf("-----------------------------------------------------------------------------------------------\n");
    printf("LRU Simulation: Average number of successful swaps = %.2f, average hit ratio = %.5f\n",
           (float)avg_lru_swaps / runs, avg_lru_hit_ratio / runs);

    int avg_lfu_swaps = 0;
    float avg_lfu_hit_ratio = 0.0;
    for (int k = 0; k < runs; k++)
    {
        avg_lfu_swaps += lfu_total_swaps[k];
        avg_lfu_hit_ratio += lfu_hit_ratios[k];
    }
    printf("-----------------------------------------------------------------------------------------------\n");
    printf("LFU Simulation: Average number of successful swaps = %.2f, average hit ratio = %.5f\n",
           (float)avg_lfu_swaps / runs, avg_lfu_hit_ratio / runs);

    int avg_rp_swaps = 0;
    float avg_rp_hit_ratio = 0.0;
    for (int k = 0; k < runs; k++)
    {
        avg_rp_swaps += rp_total_swaps[k];
        avg_rp_hit_ratio += rp_hit_ratios[k];
    }
    printf("-----------------------------------------------------------------------------------------------\n");
    printf("Random Pick Simulation: Average number of successful swaps = %.2f, average hit ratio = %.5f\n",
           (float)avg_rp_swaps / runs, avg_rp_hit_ratio / runs);

    int avg_mfu_swaps = 0;
    float avg_mfu_hit_ratio = 0.0;
    for (int k = 0; k < runs; k++)
    {
        avg_mfu_swaps += mfu_total_swaps[k];
        avg_mfu_hit_ratio += mfu_hit_ratios[k];
    }
    printf("-----------------------------------------------------------------------------------------------\n");
    printf("MFU Simulation: Average number of successful swaps = %.2f, average hit ratio = %.5f\n",
        (float)avg_mfu_swaps / runs, avg_mfu_hit_ratio / runs);

    return 0;
}
