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
void removeFromQueue(Queue *q, Process *p);
void lru(PageList *plist);
void lruSimulation(Process processes[], int numProcesses, PageList *plist, int *swaps, float *hit_ratio);

// LFU function prototypes
void lfu(PageList *plist);
void lfuSimulation(Process processes[], int numProcesses, PageList *plist, int *swapped_in, float *hit_ratio);
