#include "helper.h"

// FIFO page replacement: removes the page at the head of the free list (the one brought in first)
void fifo(PageList *plist) 
{
    if (plist == NULL || plist->head == NULL) 
    {
        printf("[ERROR] plist or plist->head is NULL in fifo\n");
        exit(EXIT_FAILURE);
    }

    Page *to_evict = NULL;
    
    // For eviction, select the page that was brought in first, ie: the one at the head of the list
    if (plist->head->process_id != -1 && plist->head->page_num != -1)
    {
        to_evict = plist->head;
    }
    else
    {
        Page* current = plist->head;
        while (current != NULL)
        {
            if (current->process_id != -1 && current->page_num != -1)
            {
                to_evict = current;
                break;
            }
            current = current->next;
        }
    }

    if (to_evict == NULL) 
    {
        printf("[ERROR] No valid FIFO page found. Exiting...\n");
        return;
    }
    printf("[DEBUG] FIFO evicting Page %d from Process %d, Brought Time: %.2f\n", 
           to_evict->page_num, to_evict->process_id, to_evict->brought_time);

    // Evict the page
    to_evict->process_id = -1;
    to_evict->page_num = -1;
    to_evict->last_referenced = 0.0f;
    to_evict->count = 0;
    to_evict->brought_time = 0.0f;
}

// FIFO Simulation: follows the same structure as LRU and LFU simulations
void fifoSimulation(Process processes[], int numProcesses, PageList *plist, int* swaps, float* hit_ratio)
{
    if (plist == NULL || plist->head == NULL || processes == NULL) 
    {
        printf("[ERROR] plist or plist->head or processes is NULL in fifoSimulation\n");
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

    for(int t = 0; t < TOTAL_DURATION; t++)
    {
        if(readyQueue->size == 0)
        {
            printf("[DEBUG] All processes have finished execution. Printing statistics...\n");
            break;
        }

        while(track_idx < TOTAL_PROCESSES && readyQueue->processes[(readyQueue->front + track_idx) % readyQueue->capacity].arrival_time <= t)
        {
            int curr_idx = (readyQueue->front + track_idx) % readyQueue->capacity;
            Process* curr_proc = &readyQueue->processes[curr_idx];

            if(checkForFreePages(plist, 4))
            {
                printf("[DEBUG] Found at least four free pages for Process %d!\n", curr_proc->id);
                Page* p = getFreePage(plist);
                if (p != NULL)
                {
                    p->process_id = curr_proc->id;
                    p->page_num = curr_proc->starting_page_num;
                    p->brought_time = (float)(t * 1.0);
                    p->count = 1;
                    p->last_referenced = (float)(t * 1.0);
                    swap_count += 1;
                }
                track_idx++;

                // Print record for process entering memory (memory map printed as a placeholder)
                printf("<%d, Process %d, Enter, Size: %d, Service Duration: %d, Memory Map: ", 
                       t, curr_proc->id, curr_proc->num_pages, curr_proc->service_time);
                printMemoryMap(plist);
                printf(">\n");
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
                int idx = (readyQueue->front + l) % readyQueue->capacity;
                Process* curr_proc = &readyQueue->processes[idx];

                if(curr_proc->service_time > 0 && ifExistsInQueue(readyQueue, curr_proc->id))
                {
                    int next_page = getNextPage(curr_proc->starting_page_num, curr_proc->num_pages);
                    curr_proc->starting_page_num = next_page;

                    if(ifExistsInMemory(plist, curr_proc->id, next_page))
                    {
                        Page* page_ptr = getPageByID(plist, curr_proc->id, next_page);
                        page_ptr->count++;
                        page_ptr->last_referenced = (float)t;
                        hit_count++;

                        printf("<%d, Process %d, Referenced Page %d, HIT, No eviction>\n", 
                               t, curr_proc->id, next_page);
                    }
                    else
                    {
                        Page* free_page = getFreePage(plist);
                        if(free_page == NULL)
                        {
                            printf("[DEBUG] No free page available for Process %d. Running FIFO replacement. Page list is:\n", curr_proc->id);
                            displayPages(plist);
                            fifo(plist);
                            printf("[DEBUG] After invoking FIFO, the page list is:\n");
                            displayPages(plist);
                            free_page = getFreePage(plist);
                            if(free_page != NULL)
                            {
                                printf("[DEBUG] Free page found after FIFO!\n");
                            }
                            else
                            {
                                printf("[DEBUG] No free page found even after FIFO. Moving on...\n");
                                continue;
                            }
                        }
                        free_page->process_id = curr_proc->id;
                        free_page->page_num = next_page;
                        free_page->brought_time = (float)(t + (0.1 * k));
                        free_page->last_referenced = (float)(t + (0.1 * k));
                        free_page->count = 1;
                        swap_count++;
                        miss_count++;

                        printf("<%d, Process %d, Referenced Page %d, MISS, Evicted: (if needed)>\n",
                               t, curr_proc->id, next_page);
                    }
                }
            }
        }

        for(int m = 0; m < track_idx; m++)
        {
            int curr_idx = (readyQueue->front + m) % readyQueue->capacity;
            Process* curr_proc = &readyQueue->processes[curr_idx];

            if(ifExistsInQueue(readyQueue, curr_proc->id))
            {
                int page_loaded = ifExistsInMemory(plist, curr_proc->id, curr_proc->starting_page_num);
                if(!page_loaded)
                {
                    Page* free_page = getFreePage(plist);
                    if(!free_page)
                    {
                        printf("[DEBUG] No free page found! Process %d is stalled.\n", curr_proc->id);
                        continue;
                    }

                    free_page->process_id = curr_proc->id;
                    free_page->page_num = curr_proc->starting_page_num;
                    free_page->brought_time = (float)t;
                    free_page->last_referenced = (float)t;
                    free_page->count = 1;
                    swap_count++;
                }

                if(curr_proc->service_time > 0)
                {
                    curr_proc->service_time -= 1;
                }

                if(curr_proc->service_time == 0)
                {
                    printf("[DEBUG] Process %d finished execution; freeing memory.\n", curr_proc->id);
                    int temp_id = curr_proc->id;

                    printf("<%d, Process %d, Exit, Size: %d, Service Duration: %d, Memory Map: ",
                           t, curr_proc->id, curr_proc->num_pages, curr_proc->service_time);
                    printMemoryMap(plist);
                    printf(">\n");

                    removeFromQueue(readyQueue, curr_proc);
                    freeMemory(plist, temp_id);
                }
            }
        }

        if (t % 10 == 0)
        {
            printf("[DEBUG] Memory state at time %d seconds:\n", t);
            printMemoryMap(plist);
        }
        usleep(900);
    }

    printf("[DEBUG] Final memory state at the end of the run:\n");
    printMemoryMap(plist);

    *swaps = swap_count;
    *hit_ratio = (hit_count + miss_count) > 0 ? (float)hit_count / (hit_count + miss_count) : 0.0f;

    printf("[DEBUG] Total number of swaps for FIFO: %d, hit ratio: %.2f\n", *swaps, *hit_ratio);
    freeQueue(readyQueue);
}
