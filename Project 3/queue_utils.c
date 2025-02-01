#include "queue_utils.h"

// Create and initialize an empty queue.
Queue *initQueue() {
    Queue *queue = (Queue *)malloc(sizeof(Queue));
    if (!queue) {
        perror("Failed to allocate queue");
        exit(EXIT_FAILURE);
    }
    queue->front = queue->rear = NULL;
    queue->size = 0;
    return queue;
}

// Enqueue a new customer at the tail.
void enqueue(Queue *queue, Customer *customer) {
    Node *node = (Node *)malloc(sizeof(Node));
    if (!node) {
        perror("Failed to allocate node");
        exit(EXIT_FAILURE);
    }
    node->customer = customer;
    node->next = NULL;
    if (queue->rear == NULL) {
        queue->front = queue->rear = node;
    } else {
        queue->rear->next = node;
        queue->rear = node;
    }
    queue->size++;
}

// Dequeue and return the customer from the head.
Customer *dequeue(Queue *queue) {
    if (queue->front == NULL)
        return NULL;
    Node *temp = queue->front;
    Customer *customer = temp->customer;
    queue->front = temp->next;
    if (queue->front == NULL)
        queue->rear = NULL;
    free(temp);
    queue->size--;
    return customer;
}

// Return nonzero if the queue is empty.
int isEmpty(Queue *queue) {
    return (queue->front == NULL);
}

// Free all memory associated with the queue.
void freeQueue(Queue *queue) {
    while (!isEmpty(queue)) {
        Customer *cust = dequeue(queue);
        free(cust);
    }
    free(queue);
}
