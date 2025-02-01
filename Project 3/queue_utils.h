#ifndef QUEUE_UTILS_H
#define QUEUE_UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* 
 * Customer structure holds the key information for a ticket buyer.
 * The id string holds the seller-specific customer id (e.g. "H001", "M101").
 */
typedef struct Customer {
    int arrival_time;         // Time (minute) at which customer arrives
    char id[8];               // Customer ID string (e.g., "H001", "M101", etc.)
    int service_start_time;   // Minute when service begins
    int service_end_time;     // Minute when service completes
    int service_duration;     // How many minutes required for service
} Customer;

/* 
 * Queue node used for our linked list queue.
 */
typedef struct Node {
    Customer *customer;
    struct Node *next;
} Node;

/* 
 * Queue structure holds pointers to the front and rear of the list.
 */
typedef struct Queue {
    Node *front;
    Node *rear;
    int size;
} Queue;

// Initializes a new empty queue.
Queue *initQueue();

// Adds a customer to the tail of the queue.
void enqueue(Queue *queue, Customer *customer);

// Removes and returns the customer at the head of the queue (or NULL if empty).
Customer *dequeue(Queue *queue);

// Returns 1 if the queue is empty; 0 otherwise.
int isEmpty(Queue *queue);

// Frees the memory allocated for the queue.
void freeQueue(Queue *queue);

#endif