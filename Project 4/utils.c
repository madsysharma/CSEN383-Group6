#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "utils.h"

int page_counts[4] = {5, 11, 17, 31};

// Generate random processes
void generateProcesses(Process *processes, int numProcesses)
{
	printf("Starting generation.\n");
	for (int i = 0; i < numProcesses; i++)
	{
		processes[i].id = i + 1;
		processes[i].num_pages = page_counts[rand() % 4];
		processes[i].arrival_time = rand() % 60;	  // Random arrival time (0–59)
		processes[i].service_time = (rand() % 5) + 1; // Random runtime (1–5)
		processes[i].starting_page_num = 0;
		if (processes[i].service_time <= 0)
		{
			printf("Invalid runtime for process %d\n", processes[i].id);
			exit(EXIT_FAILURE);
		}
	}

	// Sort processes by arrival time
	qsort(processes, numProcesses, sizeof(Process), compareByArrivalTime);
	printf("Processes generated and sorted");
}

// Comparison function for qsort (sort by arrival time)
int compareByArrivalTime(const void *a, const void *b)
{
	const Process *p1 = (const Process *)a;
	const Process *p2 = (const Process *)b;
	return p1->arrival_time - p2->arrival_time;
}

// Create a queue
Queue *createQueue(int capacity)
{
	Queue *queue = (Queue *)malloc(sizeof(Queue));
	queue->capacity = capacity;
	queue->front = 0;
	queue->rear = -1;
	queue->size = 0;
	queue->processes = (Process *)malloc(capacity * sizeof(Process));
	return queue;
}

// Check if the queue is full
int isQueueFull(Queue *queue)
{
	return queue->size == queue->capacity;
}

// Check if the queue is empty
int isQueueEmpty(Queue *queue)
{
	return queue->size == 0;
}

// Enqueue a process into the queue
void enqueue(Queue *queue, Process process)
{
	if (isQueueFull(queue))
	{
		printf("[ERROR] Queue is full. Cannot enqueue process %d.\n", process.id);
		return;
	}
	queue->rear = (queue->rear + 1) % queue->capacity;
	queue->processes[queue->rear] = process;
	queue->size++;
}

// Dequeue a process from the queue
Process dequeue(Queue *queue)
{
	if (isQueueEmpty(queue))
	{
		Process emptyProcess = {0, 0, 0, 0, 0};
		return emptyProcess;
	}
	Process process = queue->processes[queue->front];
	queue->front = (queue->front + 1) % queue->capacity;
	queue->size--;
	return process;
}

// Free queue memory
void freeQueue(Queue *queue)
{
	if (queue == NULL)
		return;

	free(queue->processes);
	free(queue);
}

// Check if process with a given ID exists in queue
int ifExistsInQueue(Queue *q, int pid)
{
	for (int i = 0; i < q->size; i++)
	{
		int idx = (q->front + i) % q->capacity;
		if (q->processes[idx].id == pid)
		{
			return 1;
		}
	}
	return 0;
}

// This function dequeues the completed process without affecting the rest of the ready queue
void removeFromQueue(Queue* q, Process* p)
{
	Queue* temp = createQueue(q->capacity);
	int found = 0;
	printf("[DEBUG] Queue before removing Process %d:\n", p->id);
	for (int i = 0; i < q->size; i++) 
	{
		int idx = (q->front + i) % q->capacity;
		printf("[DEBUG] Queue Process ID: %d\n", q->processes[idx].id);
	}
	while(!isQueueEmpty(q))
	{
		Process removed = dequeue(q);
		if(removed.id != p->id)
		{
			enqueue(temp, removed);
		}
		else
		{
			printf("[DEBUG] Process %d found. Removing it.\n", p->id);
			found = 1;
		}
	}
	if (!found) 
	{
		printf("[WARNING] Process %d not found in queue.\n", p->id);
	}
	while(!isQueueEmpty(temp))
	{
		enqueue(q,dequeue(temp));
	}
	printf("[DEBUG] Queue after removing Process %d:\n", p->id);
	for (int i = 0; i < q->size; i++) 
	{
		int idx = (q->front + i) % q->capacity;
		printf("[DEBUG] Queue Process ID: %d\n", q->processes[idx].id);
	}
	freeQueue(temp);
}

// Initialize the free page list
void initPageList(PageList *plist)
{
	if (plist == NULL)
	{
		printf("[ERROR] plist is NULL in initPageList\n");
		exit(EXIT_FAILURE);
	}

	plist->head = (Page *)malloc(sizeof(Page));
	if (plist->head == NULL)
	{
		printf("[ERROR] Memory allocation failed for plist->head\n");
		exit(EXIT_FAILURE);
	}

	Page *current = plist->head;
	for (int i = 0; i < TOTAL_PAGE_REFS; i++)
	{
		current->process_id = -1;
		current->page_num = -1;
		current->next = NULL;

		if (i < TOTAL_PAGE_REFS - 1)
		{
			current->next = (Page *)malloc(sizeof(Page));
			if (current->next == NULL)
			{
				printf("[ERROR] Memory allocation failed for a Page\n");
				exit(EXIT_FAILURE);
			}
			current = current->next;
		}
	}
}

// Display all the pages in the free page list
void displayPages(PageList *plist)
{
	Page *current = plist->head;
	int counter = 0;
	while (current != NULL)
	{
		if (current->process_id > 0)
		{
			printf("--- PID:%03d Page Number:%03d Count:%03d Last Referenced:%02f ---\n", current->process_id, current->page_num, current->count, current->last_referenced);
		}
		else
		{
			printf("---                  ---\n");
		}
		counter++;
		if ((counter % 10) == 0)
		{
			printf("\n");
		}
		current = current->next;
	}
}

// Check if there is a free page available
int checkForFreePages(PageList *plist, int count)
{
	Page *current = plist->head;
	while (current)
	{
		if (current->process_id == -1)
		{
			count -= 1;
		}
		if (count == 0)
		{
			return 1;
		}
		current = current->next;
	}
	return 0;
}

// Return a free page
Page *getFreePage(PageList *plist)
{
	if (plist == NULL || plist->head == NULL)
	{
		printf("[ERROR] getFreePage called on NULL PageList\n");
		exit(EXIT_FAILURE);
	}

	Page *current = plist->head;
	while (current != NULL)
	{
		if (current->process_id < 0)
		{
			printf("[DEBUG] Free page available: pid %d, page_num %d, count %d, last referenced %.2f\n", current->process_id, current->page_num, current->count, current->last_referenced);
			return current;
		}
		current = current->next;
	}
	return NULL;
}

// Free up memory occupied by a process
void freeMemory(PageList *plist, int pid)
{
	if (plist == NULL || plist->head == NULL)
	{
		printf("[ERROR] PageList is NULL\n");
		return;
	}

	Page *current = plist->head;
	int freed_count = 0;

	while (current != NULL)
	{
		if (current->process_id == pid)
		{
			printf("[DEBUG] Freeing Page: Process ID = %d, Page Number = %d\n", current->process_id, current->page_num);
			current->process_id = -1;
			current->page_num = -1;
			freed_count++;
		}
		current = current->next;
	}

	printf("[DEBUG] Total Pages Freed: %d for Process ID: %d\n", freed_count, pid);
}

// Return a page matching the process ID and page number
Page *getPageByID(PageList *plist, int pid, int pg_num)
{
	Page *current = plist->head;
	while (current != NULL)
	{
		if (current->process_id == pid && current->page_num == pg_num)
		{
			printf("[DEBUG] Accessing Page %d for Process %d, Count: %d, Last Referenced: %.2f\n",
				   pg_num, pid, current->count, current->last_referenced);
			return current;
		}
		current = current->next;
	}
	return NULL;
}

// Check if the page actually exists in memory
int ifExistsInMemory(PageList *plist, int pid, int pg_num)
{
	Page *current = plist->head;
	printf("Requested pid: %d\n", pid);
	while (current != NULL)
	{
		if (current->process_id == pid && current->page_num == pg_num)
		{
			if (current->process_id == -1)
			{
				printf("[DEBUG] Not sending back freed page.");
				return 0;
			}
			else
			{
				printf("[DEBUG] Page FOUND in memory: Process %d, Page %d, Last Referenced: %.2f\n", pid, pg_num, current->last_referenced);
				return 1;
			}
		}
		current = current->next;
	}
	printf("[DEBUG] Page NOT found in memory: Process %d, Page %d\n", pid, pg_num);
	return 0;
}

// Return the next page for the requesting process
int getNextPage(int curr_page_num, int max_page_size)
{
	int next_page = rand() % 10;
	if (next_page < 7)
	{
		next_page = curr_page_num + (rand() % 3) - 1;
	}
	else
	{
		next_page = rand() % max_page_size;
		while (abs(next_page - curr_page_num) <= 1)
		{
			next_page = rand() % max_page_size;
		}
	}
	if (next_page < 0)
	{
		next_page = 0;
	}
	if (next_page >= TOTAL_PAGE_REFS)
	{
		next_page = max_page_size - 1;
	}
	return next_page;
}

// Freeing the page list
void freePageList(PageList *plist)
{
	if (plist == NULL)
	{
		printf("[ERROR] freePageList() called on NULL PageList\n");
		return;
	}

	Page *current = plist->head;
	while (current != NULL)
	{
		Page *temp = current;
		current = current->next;
		free(temp); // Free the Page struct itself
	}
	free(plist);
}

// Printing the memory map
void printMemoryMap(PageList *plist)
{
	char mem_map[501];
	memset(mem_map, '.', sizeof(mem_map));
	Page *current = plist->head;
	size_t pos = 0;
	while (current != NULL && pos < sizeof(mem_map) - 5)
	{
		if (current->process_id != -1)
		{
			pos += snprintf(&mem_map[pos], sizeof(mem_map) - pos, "%d ", current->process_id);
		}
		else
		{
			pos += snprintf(&mem_map[pos], sizeof(mem_map) - pos, ". ");
		}
		current = current->next;
	}
	mem_map[pos] = '\0';
	printf("[MEMORY MAP] %s\n", mem_map);
}
