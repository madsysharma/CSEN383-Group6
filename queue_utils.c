#include <stdio.h>
#include <stdlib.h>
#include "queue_utils.h"

// Create a queue
Queue* createQueue(int capacity) {
    Queue* queue = (Queue*)malloc(sizeof(Queue));
    queue->capacity = capacity;
    queue->front = 0;
    queue->rear = -1;
    queue->size = 0;
    queue->processes = (Process*)malloc(capacity * sizeof(Process));
    return queue;
}

// Check if the queue is full
int isQueueFull(Queue* queue) {
    return queue->size == queue->capacity;
}

// Check if the queue is empty
int isQueueEmpty(Queue* queue) {
    return queue->size == 0;
}

// Enqueue a process into the queue
void enqueue(Queue* queue, Process process) {
    if (isQueueFull(queue)) {
        printf("Queue is full. Cannot enqueue process %c.\n", process.name);
        return;
    }
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->processes[queue->rear] = process;
    queue->size++;
}

// Dequeue a process from the queue
Process dequeue(Queue* queue) {
    if (isQueueEmpty(queue)) {
        Process emptyProcess = {'\0', 0, 0, 0, -1, 0}; // Return an empty process
        return emptyProcess;
    }
    Process process = queue->processes[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;
    return process;
}

// Free queue memory
void freeQueue(Queue* queue) {
    free(queue->processes);
    free(queue);
}

