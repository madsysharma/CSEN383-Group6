#ifndef QUEUE_UTILS_H
#define QUEUE_UTILS_H

// Queue structure
typedef struct Queue {
    int front;          // Front index
    int rear;           // Rear index
    int size;           // Current size
    int capacity;       // Maximum capacity
    void** customers;   // Array of customers
} Queue;

// Queue function prototypes
Queue* createQueue(int capacity);
int isQueueFull(Queue* queue);
int isQueueEmpty(Queue* queue);
void enqueue(Queue* queue, void* customer);
void* dequeue(Queue* queue);
void freeQueue(Queue* queue);

#endif

