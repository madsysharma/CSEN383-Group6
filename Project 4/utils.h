#ifndef UTILS_H
#define UTILS_H
#endif

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define TOTAL_PROCESSES 150
#define TOTAL_PAGE_REFS 100
#define TOTAL_DURATION 60

typedef struct page {
	int process_id;
	int page_num;
	char* mem_map;
	struct page* next;
	float brought_time;
	float last_referenced;
	int count;
} Page;

typedef struct {
	Page* head;
} PageList;

typedef struct {
	int id;
	int num_pages;
	int arrival_time;
	int service_time;
	int starting_page_num;
} Process;

typedef struct {
	Process* processes;
	int front;
	int rear;
	int size;
	int capacity;
} Queue;

// Paging algorithm helper functions
void initPageList(PageList* plist);
void displayPages(PageList* plist);
int checkForFreePages(PageList* plist, int count);
Page* getFreePage(PageList* plist);
void freeMemory(PageList* plist, int pid);
Page* getPageByID(PageList* plist, int pid, int pg_num);
int ifExistsInMemory(PageList* plist, int pid, int pg_num);
int getNextPage(int curr_page_num, int max_page_size);
void freePageList(PageList* plist);

// Queue functions
Queue* createQueue(int capacity);
int isQueueFull(Queue* queue);
int isQueueEmpty(Queue* queue);
void enqueue(Queue* queue, Process process);
Process dequeue(Queue* queue);
void freeQueue(Queue* queue);
int ifExistsInQueue(Queue* queue, int pid);

// Process generation function
void generateProcesses(Process* processes, int numProcesses);
int compareByArrivalTime(const void* a, const void* b);
