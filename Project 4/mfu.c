#include "helper.h"
#include <limits.h>

// MFU page replacement: finds the page with the highest reference count and evicts it
void mfu(PageList *plist)
{
    if (plist == NULL || plist->head == NULL)
    {
        printf("[ERROR] plist or plist->head is NULL in mfu\n");
        exit(EXIT_FAILURE);
    }

    Page *current = plist->head;
    Page *to_evict = NULL;
    int max_count = -1;

    // For eviction, select the page with the highest reference count
    while (current != NULL)
    {
        if (current->process_id != -1 && current->page_num != -1 && current->count > max_count)
        {
            max_count = current->count;
            to_evict = current;
        }
        current = current->next;
    }

    if (to_evict == NULL)
    {
        printf("[ERROR] No valid MFU page found. Exiting...\n");
        return;
    }

    printf("[DEBUG] MFU Evicting Page %d from Process %d, Count: %d\n",
           to_evict->page_num, to_evict->process_id, to_evict->count);

    // Evict the page
    to_evict->process_id = -1;
    to_evict->page_num = -1;
    to_evict->count = 0;
    to_evict->last_referenced = 0.0;
}

// MFU simulation: runs for TOTAL_DURATION seconds and simulates page references
void mfuSimulation(Process processes[], int numProcesses, PageList *plist, int *swapped_in, float *hit_ratio)
{
    if (plist == NULL || plist->head == NULL || processes == NULL)
    {
        printf("[ERROR] plist or plist->head or processes is NULL in mfuSimulation\n");
        exit(EXIT_FAILURE);
    }

    int swap_count = 0;
    int hit_count = 0;
    int miss_count = 0;
    int track_idx = 0;

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

        // Check for new process arrivals
        while (track_idx < TOTAL_PROCESSES &&
               readyQueue->processes[(readyQueue->front + track_idx) % readyQueue->capacity].arrival_time <= t)
        {
            int curr_idx = (readyQueue->front + track_idx) % readyQueue->capacity;
            Process *curr_proc = &readyQueue->processes[curr_idx];

            if (checkForFreePages(plist, 4))
            {
                printf("[DEBUG] Found at least four free pages for Process %d!\n", curr_proc->id);
                Page *p = getFreePage(plist);
                p->process_id = curr_proc->id;
                p->page_num = curr_proc->starting_page_num;
                p->brought_time = (float)t;
                p->last_referenced = (float)t;
                p->count = 1;
                swap_count++;
                track_idx++;

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

        // Simulate page references
        for (int k = 0; k < 10; k++)
        {
            for (int l = 0; l < track_idx; l++)
            {
                int idx = (readyQueue->front + l) % readyQueue->capacity;
                Process *curr_proc = &readyQueue->processes[idx];

                if (curr_proc->service_time > 0 && ifExistsInQueue(readyQueue, curr_proc->id))
                {
                    int next_page = getNextPage(curr_proc->starting_page_num, curr_proc->num_pages);
                    curr_proc->starting_page_num = next_page;

                    if (ifExistsInMemory(plist, curr_proc->id, next_page))
                    {
                        Page *page_ptr = getPageByID(plist, curr_proc->id, next_page);
                        page_ptr->count++;
                        page_ptr->last_referenced = (float)t;
                        hit_count++;

                        printf("<%d, Process %d, Referenced Page %d, HIT, No eviction>\n",
                               t, curr_proc->id, next_page);
                    }
                    else
                    {
                        Page *free_page = getFreePage(plist);
                        if (free_page == NULL)
                        {
                            printf("[DEBUG] No free page available. Running MFU replacement.\n");
                            mfu(plist);
                            free_page = getFreePage(plist);
                        }

                        if (free_page == NULL)
                        {
                            printf("[DEBUG] No free page found even after MFU replacement. Skipping this reference.\n");
                            continue;
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

        // Process execution and cleanup
        for (int m = 0; m < track_idx; m++)
        {
            int curr_idx = (readyQueue->front + m) % readyQueue->capacity;
            Process *curr_proc = &readyQueue->processes[curr_idx];

            if (ifExistsInQueue(readyQueue, curr_proc->id))
            {
                int page_loaded = ifExistsInMemory(plist, curr_proc->id, curr_proc->starting_page_num);
                if (!page_loaded)
                {
                    Page *free_page = getFreePage(plist);
                    if (!free_page)
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

                if (curr_proc->service_time > 0)
                {
                    curr_proc->service_time -= 1;
                }

                if (curr_proc->service_time == 0)
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

    *swapped_in = swap_count;
    *hit_ratio = (hit_count + miss_count) > 0 ? (float)hit_count / (hit_count + miss_count) : 0.0f;

    printf("[DEBUG] Total number of swaps for MFU: %d, hit ratio: %.2f\n", swap_count, *hit_ratio);
    freeQueue(readyQueue);
}
