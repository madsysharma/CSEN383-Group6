#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <string.h>
#include "process_utils.h"
#include "queue_utils.h"

void fcfs(Process processes[], int numProcesses, float* avgTurnaroundTime, float* avgWaitingTime, float* avgResponseTime, float* throughput);
void sjf(Process* processes, int numProcesses, float* avgTurnaroundTime, float* avgWaitingTime, float* avgResponseTime, float* throughput);
Process* getSRTFProcess(Queue* q);
void removeFromQueue(Queue* q, Process* p);
void srtf(Process processes[], int numProcesses, float* avgTurnaroundTime, float* avgWaitingTime, float* avgResponseTime, float* throughput);
void roundRobin(Process processes[], int numProcesses, float* avgTurnaroundTime, float* avgWaitingTime, float* avgResponseTime, float* throughput);
void simulateHPFNonPreemptive(int runIndex, Process processes[], int numProcesses, float* avgTurnaroundTime, float* avgWaitingTime, float* avgResponseTime, float* throughput);
void runNonPreemptive(Process *processes, int numProcesses, float* avgTurnaroundTime, float* avgWaitingTime, float* avgResponseTime, float* throughput);
void simulateHPFPreemptive(int runIndex, Process processes[], int numProcesses, float* avgTurnaroundTime, float* avgWaitingTime, float* avgResponseTime, float* throughput);
void runPreemptive(Process *processes, int numProcesses, float* avgTurnaroundTime, float* avgWaitingTime, float* avgResponseTime, float* throughput);
void printRunStatisticsNonPreemptive(Process *processes, int numProcesses, int totalRunTime, Timeline *timeline, float* avgTurnaroundTime, float* avgWaitingTime, float* avgResponseTime, float* throughput);
void printRunStatisticsPreemptive(Process *processes, int numProcesses, int totalRunTime, Timeline *timeline, float* avgTurnaroundTime, float* avgWaitingTime, float* avgResponseTime, float* throughput);