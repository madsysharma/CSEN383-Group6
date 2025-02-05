#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

// Generate random processes
void generateProcesses(Process* processes, int numProcesses) {
    char processID = 'A';
    for (int i = 0; i < numProcesses; i++) {
        processes[i].id = processID;
        if (processID == 'Z')
        {
            processID = 'a' - 1;
        }
        processID++;
        processes[i].arrival_time = rand() % 60;       // Random arrival time (0–59)
        processes[i].service_time = (rand() % 5) + 1;     // Random runtime (1–5)
        if (processes[i].service_time <= 0) 
        {
            fprintf(stderr, "Invalid runtime for process %c\n", processes[i].id);
            exit(EXIT_FAILURE);
        }
    }

    // Sort processes by arrival time
    qsort(processes, numProcesses, sizeof(Process), compareByArrivalTime);
}

// Comparison function for qsort (sort by arrival time)
int compareByArrivalTime(const void* a, const void* b) {
    const Process* p1 = (const Process*)a;
    const Process* p2 = (const Process*)b;
    return p1->arrival_time - p2->arrival_time;
}

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
        printf("Queue is full. Cannot enqueue process %c.\n", process.id);
        return;
    }
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->processes[queue->rear] = process;
    queue->size++;
}

// Dequeue a process from the queue
Process dequeue(Queue* queue) {
    if (isQueueEmpty(queue)) {
      	Process emptyProcess = {'\0', 0, 0, 0}; // Return an empty process
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

// Initialize the free page list
void initPageList(PageList* plist)
{
	plist->head = malloc(sizeof(Page));
	Page* current = plist->head;
	for(int i = 0; i < TOTAL_PAGE_REFS; i++)
	{
		current->process_id = '\0';
		current->page_num = -1;
		current->next = NULL;
		if(i < 99)
		{
			current->next = malloc(sizeof(Page));
			current = current->next;
		}
	}
}

// Display all the pages in the free page list
void displayPages(PageList* plist)
{
	Page* current = plist->head;
	int counter = 0;
	while(current != NULL)
	{
		if(current->count == 0)
		{
			printf("Page %d\t.|",counter);
			counter++;
		}
		else
		{
			printf("\n");
			if(current->process_id != '\0')
			{
				printf("PID: %c\tCount: %d\tLast referenced: %.2f|\n", current->process_id, current->count, current->last_referenced);
			}
			else
			{
				printf("Count: %d\tLast referenced: %.2f|\n", current->process_id, current->count, current->last_referenced);
			}
		}
		if(counter % 10 == 0)
		{
			printf("\n");
		}
		current = current->next;
	}
}

// Check if there is a free page available
int checkForFreePages(PageList* plist, int count)
{
	Page* current = plist->head;
	while(current != NULL)
	{
		if(current->process_id != '\0')
		{
			count-=1;
		}
		if(count == 0)
		{
			return 1;
		}
		current = current->next;
	}
	return 0;
}

// Return a free page
Page* getFreePage(PageList* plist)
{
	Page* current = plist->head;
	while(current != NULL)
	{
		if(current->process_id != '\0')
		{
			return current;
		}
		current = current->next;
	}
	return NULL;
}

// Free up memory occupied by a process
void freeMemory(PageList* plist, char pid)
{
	Page* current = plist->head;
	while(current != NULL)
	{
		if(current->process_id == pid)
		{
			current->process_id = '\0';
			current->page_num = -1;
		}
		current = current->next;
	}
}

// Return a page matching the process ID and page number
Page* getPageByID(PageList* plist, char pid, int pg_num)
{
	Page* current = plist->head;
	while(current != NULL)
	{
		if(current->process_id == pid && current->page_num == pg_num)
		{
			return current;
		}
		current = current->next;
	}
	return NULL;
}

// Check if the page actually exists in memory
int ifExistsInMemory(PageList* plist, char pid, int pg_num)
{
	Page* current = plist->head;
	while(current != NULL)
	{
		if(current->process_id == pid && current->page_num == pg_num)
		{
			return 1;
		}
		current = current->next;
	}
	return 0;
}

// Return the next page for the requesting process
int getNextPage(int curr_page_num, int max_page_size)
{
	int next_page = rand() % 10;
	if(next_page < 7)
	{
		next_page = curr_page_num + (rand()%3) - 1;
		if(next_page < 0)
		{
			next_page = 0;
		}
	}
	else
	{
		next_page = rand() % max_page_size;
		while(abs(next_page - curr_page_num) <= 1)
		{
			next_page = rand() % max_page_size;
		}
	}
	if(next_page < 0)
	{
		next_page = 0;
	}
	if(next_page >= 100)
	{
		next_page = max_page_size - 1;
	}
	return next_page;
}

// Freeing the page list
void freePageList(PageList* plist)
{
	Page* current = plist->head;
	while(current != NULL)
	{
		Page* temp = current;
		current = current->next;
		free(temp);
	}
}