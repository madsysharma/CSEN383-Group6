#include "helper.h"

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



void lru(PageList *plist) 
{
	printf("Starting LRU\n");
	if (plist == NULL || plist->head == NULL) 
	{
		printf("[ERROR] plist or plist->head is NULL in lru\n");
		exit(EXIT_FAILURE);
	}

	Page* current = plist->head;
	Page* to_remove = NULL;
	float lru_time = current->last_referenced;
	while (current != NULL) 
	{
		if (current->last_referenced < lru_time) 
		{
			to_remove = current;
			lru_time = (float)current->last_referenced;
		}
		current = current->next;
	}

	if (to_remove == NULL) 
	{
		printf("[ERROR] No valid LRU page found. Exiting...\n");
		return;
	}
	printf("[DEBUG] Evicting Page %d from Process %d, Last Referenced: %.2f\n", to_remove->page_num, to_remove->process_id, to_remove->last_referenced);

	to_remove->process_id = -1;
	to_remove->page_num = -1;
}




void lruSimulation(Process processes[], int numProcesses, PageList *plist, int* swaps, float* hit_ratio)
{
	if (plist == NULL || plist->head == NULL || processes == NULL) 
	{
		printf("[ERROR] plist or plist->head or processes is NULL in lruSimulation\n");
		exit(EXIT_FAILURE);
	}
	int swap_count = 0;
	int hit_count = 0;
	int miss_count = 0;
	int track_idx = 0;
	Queue *readyQueue = createQueue(numProcesses);
	for(int i = 0; i < numProcesses; i++)
	{
		enqueue(readyQueue, processes[i]);
	}
	for(int t = 0; t < TOTAL_DURATION ; t++)
	{
		if(readyQueue->size == 0)
		{
			printf("[DEBUG] All processes have finished execution. Printing statistics...\n");
			break;
		}
		while(track_idx < TOTAL_PROCESSES && readyQueue->processes[(readyQueue->front + track_idx) % (readyQueue->capacity)].arrival_time <= t)
		{
			int curr_idx = (readyQueue->front + track_idx) % (readyQueue->capacity);
			Process* curr_proc = &readyQueue->processes[curr_idx];
			if(checkForFreePages(plist, 4) == 1)
			{
            	// If pages are present, load into memory
				printf("[DEBUG] At least four free pages found!\n");
				Page *p = getFreePage(plist);
				p->process_id = curr_proc->id;
				p->page_num = curr_proc->starting_page_num;
				p->brought_time = (float)(t * 1.0);
				p->count = 1;
				p->last_referenced = (float)(t * 1.0);
				swap_count += 1;
				track_idx += 1;
			}
			else
			{
				break;
			}
		}
		for(int k = 0; k < 10; k++)
		{
			for(int l = 0; l < track_idx; l++)
			{
				int idx = (readyQueue->front + l) % (readyQueue->capacity);
				if(readyQueue->processes[idx].service_time > 0 && ifExistsInQueue(readyQueue, readyQueue->processes[idx].id))
				{
					readyQueue->processes[idx].starting_page_num = getNextPage(readyQueue->processes[idx].starting_page_num, readyQueue->processes[idx].num_pages);

					if(ifExistsInMemory(plist, readyQueue->processes[idx].id, readyQueue->processes[idx].starting_page_num))
					{
						Page* ptr = getPageByID(plist, readyQueue->processes[idx].id, readyQueue->processes[idx].starting_page_num);
						if (ptr == NULL)
						{
							printf("[ERROR]: Page not found in memory but expected.\n");
							return;
						}
						ptr->count += 1;
						ptr->last_referenced = (float)(t * 1.0);
						hit_count += 1;
						continue;
					}
					else
					{
						Page* free_page = getFreePage(plist);
						if(free_page == NULL)
						{
							printf("[DEBUG] No free pages found, running LRU. Page list is:\n");
							displayPages(plist);
							lru(plist);
							printf("[DEBUG] After invoking LRU, the page list is:\n");
							displayPages(plist);
							free_page = getFreePage(plist);
							if(free_page != NULL)
							{
								printf("[DEBUG] Free page found after LRU!\n");
							}
							else
							{
								printf("[DEBUG] No free page found even after LRU. Moving on...\n");
								continue;
							}
						}
						free_page->process_id = readyQueue->processes[idx].id;
						free_page->page_num = readyQueue->processes[idx].starting_page_num;
						free_page->brought_time = (float)(t + (0.1 * k));
						free_page->last_referenced = (float)(t + (0.1 * k));
						free_page->count = 0;
						swap_count += 1;
						miss_count += 1;
					}
				}
			}
		}
		printf("[DEBUG] Starting to execute processes at timestamp %d units.\n",t); 
		for(int m = 0; m < track_idx; m++)
		{
			if(readyQueue->size > 0)
			{
				int curr_idx = (readyQueue->front + m) % (readyQueue->capacity);
				Process* curr_proc = &readyQueue->processes[curr_idx];
				if(ifExistsInQueue(readyQueue, curr_proc->id))
				{
					int page_loaded = ifExistsInMemory(plist, curr_proc->id, curr_proc->starting_page_num);
					if(!page_loaded)
					{
						Page* free_page = getFreePage(plist);
						if(free_page == NULL)
						{
							printf("[DEBUG] No free page found! Process %d is stalled.\n", curr_proc->id);
							continue;
						}
						printf("[DEBUG] Free page found!\n");
						free_page->process_id = curr_proc->id;
						free_page->page_num = curr_proc->starting_page_num;
						free_page->brought_time = (float)(t * 1.0);
						free_page->last_referenced = (float)(t * 1.0);
						free_page->count = 1;
						page_loaded = 1;
						swap_count += 1;
					}
					if(curr_proc->service_time > 0 && page_loaded == 1)
					{
						curr_proc->service_time -= 1;
					}
					if(curr_proc->service_time == 0)
					{
						if (ifExistsInQueue(readyQueue, curr_proc->id)) {
							printf("[DEBUG] Process %d is done, freeing memory from it.\n", curr_proc->id);
							int temp_id = curr_proc->id;
							removeFromQueue(readyQueue, curr_proc);
							freeMemory(plist, temp_id);
						}
						else
						{
							printf("[ERROR] Process %d already removed from queue!\n", curr_proc->id);
							continue;
						}
					}
				}
			}
		}
		if (t % 10 == 0)
		{
			printf("[DEBUG] Memory state at time %d seconds:\n", t / 10);
			printMemoryMap(plist); 
		}
		usleep(900);
	}
	printf("[DEBUG] Memory state at the end of the run:\n");
    printMemoryMap(plist); 
	*swaps = swap_count;
	*hit_ratio = (float)hit_count / (hit_count + miss_count);
	printf("[DEBUG] Total number of sucessful swaps: %d, hit ratio: %.2f\n", *swaps, *hit_ratio);
}
