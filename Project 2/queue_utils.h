#ifndef QUEUE_UTILS_H
#define QUEUE_UTILS_H

#include "process_utils.h"

// Queue structure
typedef struct Queue {
    Process* processes; // Array of processes
    int front;          // Front index
    int rear;           // Rear index
    int size;           // Current size
    int capacity;       // Maximum capacity
} Queue;

// Queue function prototypes
Queue* createQueue(int capacity);
int isQueueFull(Queue* queue);
int isQueueEmpty(Queue* queue);
void enqueue(Queue* queue, Process process);
Process dequeue(Queue* queue);
void freeQueue(Queue* queue);

#endif

