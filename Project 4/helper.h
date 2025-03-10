#ifndef HELPER_H
#define HELPER_H
#endif
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <float.h>
#include "utils.h"

// LRU function prototypes
void lru(PageList *plist);
void lruSimulation(Process processes[], int numProcesses, PageList *plist, int *proc_swaps, int *total_swaps, float *hit_ratio);

// LFU function prototypes
void lfu(PageList *plist);
void lfuSimulation(Process processes[], int numProcesses, PageList *plist, int *proc_swaps, int *total_swaps, float *hit_ratio);

// Random Pick prototypes
void randomPick(PageList* plist);
void randomPickSimulation(Process processes[], int numProcesses, PageList* plist, int* proc_swaps, int* total_swaps, float* hit_ratio);

// FIFO function prototypes
void fifo(PageList *plist);
void fifoSimulation(Process processes[], int numProcesses, PageList *plist, int* proc_swaps, int* total_swaps, float* hit_ratio);

// MFU function prototypes
void mfu(PageList *plist);
void mfuSimulation(Process processes[], int numProcesses, PageList *plist, int* proc_swaps, int* total_swaps, float* hit_ratio);
