#include "helper.h"
#include <limits.h>

// LFU page replacement: finds the page with the smallest count and evicts it.
void lfu(PageList *plist)
{
    if (plist == NULL || plist->head == NULL)
    {
        printf("[ERROR] plist or plist->head is NULL in lfu\n");
        exit(EXIT_FAILURE);
    }
    Page *current = plist->head;
    Page *to_evict = NULL;
    int min_count = INT_MAX;

    // For eviction, select the page with the lowest reference count
    while (current != NULL)
    {
        if (current->process_id != -1 && current->page_num != -1 && current->count < min_count)
        {
            min_count = current->count;
            to_evict = current;
        }
        current = current->next;
    }
    if (to_evict == NULL)
    {
        printf("[ERROR] No valid LFU page found. Exiting...\n");
        return;
    }
    printf("[DEBUG] LFU Evicting Page %d from Process %d, Count: %d\n",
           to_evict->page_num, to_evict->process_id, to_evict->count);
    // Evict the page
    to_evict->process_id = -1;
    to_evict->page_num = -1;
    to_evict->count = 0;
    to_evict->last_referenced = 0.0;
}

// LFU simulation: runs for TOTAL_DURATION seconds (1 minute) and simulates page references.
// It uses the LFU algorithm when no free page is available.
// The simulation prints records for each memory reference and counts how many processes
// were successfully swapped in.
void lfuSimulation(Process processes[], int numProcesses, PageList *plist, int *swapped_in, float *hit_ratio)
{
    if (plist == NULL || plist->head == NULL || processes == NULL)
    {
        printf("[ERROR] plist or plist->head or processes is NULL in lfuSimulation\n");
        exit(EXIT_FAILURE);
    }
    int swap_count = 0;               // Counts page swap events (page-ins)
    int hit_count = 0;                // Counts page hits
    int miss_count = 0;               // Counts page misses
    int process_swapped_in_count = 0; // Counts processes that have been swapped in initially
    int track_idx = 0;                // Number of processes currently swapped in

    Queue *readyQueue = createQueue(numProcesses);
    for (int i = 0; i < numProcesses; i++)
    {
        enqueue(readyQueue, processes[i]);
    }

    for (int t = 0; t < TOTAL_DURATION; t++)
    {
        if (readyQueue->size == 0)
        {
            printf("[DEBUG] All processes have finished execution. Printing statistics...\n");
            break;
        }
        // Check for new process arrivals; a process is swapped in if at least 4 free pages are available.
        while (track_idx < TOTAL_PROCESSES &&
               readyQueue->processes[(readyQueue->front + track_idx) % readyQueue->capacity].arrival_time <= t)
        {
            int curr_idx = (readyQueue->front + track_idx) % readyQueue->capacity;
            Process *curr_proc = &readyQueue->processes[curr_idx];
            if (checkForFreePages(plist, 4) == 1)
            {
                printf("[DEBUG] At least four free pages found for Process %d!\n", curr_proc->id);
                Page *p = getFreePage(plist);
                if (p != NULL)
                {
                    p->process_id = curr_proc->id;
                    p->page_num = curr_proc->starting_page_num;
                    p->brought_time = (float)(t * 1.0);
                    p->count = 1;
                    p->last_referenced = (float)(t * 1.0);
                    swap_count += 1;
                    process_swapped_in_count += 1;
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
        // Simulate page references for each swapped-in process.
        // Here we run 10 iterations per time unit; each iteration simulates one reference.
        for (int k = 0; k < 10; k++)
        {
            for (int l = 0; l < track_idx; l++)
            {
                int idx = (readyQueue->front + l) % readyQueue->capacity;
                Process *curr_proc = &readyQueue->processes[idx];
                if (curr_proc->service_time > 0 && ifExistsInQueue(readyQueue, curr_proc->id))
                {
                    // Determine next page reference using the locality of reference algorithm.
                    int next_page = getNextPage(curr_proc->starting_page_num, curr_proc->num_pages);
                    curr_proc->starting_page_num = next_page;
                    // Check if the page is already in memory.
                    if (ifExistsInMemory(plist, curr_proc->id, next_page))
                    {
                        Page *page_ptr = getPageByID(plist, curr_proc->id, next_page);
                        if (page_ptr == NULL)
                        {
                            printf("[ERROR] Expected page not found in memory for Process %d, Page %d.\n",
                                   curr_proc->id, next_page);
                            continue;
                        }
                        page_ptr->count++;
                        page_ptr->last_referenced = (float)t;
                        hit_count++;
                        // Print record for a page hit.
                        printf("<%d, Process %d, Referenced Page %d, HIT, No eviction>\n",
                               t, curr_proc->id, next_page);
                        continue;
                    }
                    else
                    {
                        // Page miss: try to get a free page.
                        Page *free_page = getFreePage(plist);
                        if (free_page == NULL)
                        {
                            printf("[DEBUG] No free page available for Process %d. Running LFU replacement.\n", curr_proc->id);
                            // For debugging, identify the candidate for eviction.
                            Page *current = plist->head;
                            Page *candidate = NULL;
                            int min_count = INT_MAX;
                            while (current != NULL)
                            {
                                if (current->process_id != -1 && current->count < min_count)
                                {
                                    min_count = current->count;
                                    candidate = current;
                                }
                                current = current->next;
                            }
                            if (candidate != NULL)
                            {
                                printf("[DEBUG] LFU candidate for eviction: Process %d, Page %d, Count: %d\n",
                                       candidate->process_id, candidate->page_num, candidate->count);
                            }
                            lfu(plist);
                            free_page = getFreePage(plist);
                            if (free_page == NULL)
                            {
                                printf("[DEBUG] No free page found even after LFU replacement. Skipping this reference.\n");
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
                        // Print record for a page miss and page-in event.
                        printf("<%d, Process %d, Referenced Page %d, MISS, Evicted: (if needed)>\n",
                               t, curr_proc->id, next_page);
                    }
                }
            }
        }
        // Process execution: decrement service times and remove finished processes.
        for (int m = 0; m < track_idx; m++)
        {
            if (readyQueue->size > 0)
            {
                int curr_idx = (readyQueue->front + m) % readyQueue->capacity;
                Process *curr_proc = &readyQueue->processes[curr_idx];
                if (ifExistsInQueue(readyQueue, curr_proc->id))
                {
                    int page_loaded = ifExistsInMemory(plist, curr_proc->id, curr_proc->starting_page_num);
                    if (!page_loaded)
                    {
                        Page *free_page = getFreePage(plist);
                        if (free_page == NULL)
                        {
                            printf("[DEBUG] No free page found! Process %d is stalled.\n", curr_proc->id);
                            continue;
                        }
                        printf("[DEBUG] Allocating free page for stalled Process %d!\n", curr_proc->id);
                        free_page->process_id = curr_proc->id;
                        free_page->page_num = curr_proc->starting_page_num;
                        free_page->brought_time = (float)t;
                        free_page->last_referenced = (float)t;
                        free_page->count = 1;
                        page_loaded = 1;
                        swap_count++;
                    }
                    if (curr_proc->service_time > 0 && page_loaded == 1)
                    {
                        curr_proc->service_time -= 1;
                    }
                    if (curr_proc->service_time == 0)
                    {
                        if (ifExistsInQueue(readyQueue, curr_proc->id))
                        {
                            printf("[DEBUG] Process %d has finished execution; freeing its memory.\n", curr_proc->id);
                            int temp_id = curr_proc->id;
                            // Print process exit record.
                            printf("<%d, Process %d, Exit, Size: %d, Service Duration: %d, Memory Map: ",
                                   t, curr_proc->id, curr_proc->num_pages, curr_proc->service_time);
                            printMemoryMap(plist);
                            printf(">\n");
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
    printf("[DEBUG] Final memory state at the end of the run:\n");
    printMemoryMap(plist);
    *swapped_in = swap_count;
    *hit_ratio = (hit_count + miss_count) > 0 ? (float)hit_count / (hit_count + miss_count) : 0.0f;
    printf("[DEBUG] Total number of swaps for LFU: %d, hit ratio: %.2f\n", swap_count, *hit_ratio);
    freeQueue(readyQueue);
}
