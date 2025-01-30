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
    queue->customers = (void**)malloc(capacity * sizeof(void*));
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
void enqueue(Queue* queue, void* customer) {
    if (isQueueFull(queue)) {
        printf("Queue is full.\n");
        return;
    }
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->customers[queue->rear] = customer;
    queue->size++;
}

// Dequeue a process from the queue
void* dequeue(Queue* queue) {
    if (isQueueEmpty(queue)) {
        return NULL;
    }
    void* customer = queue->customers[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;
    return customer;
}

// Free queue memory
void freeQueue(Queue* queue) {
    free(queue->customers);
    free(queue);
}

