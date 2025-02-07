
#include "helper.h"

//   Randomly pick one page from all pages currently in use and evict it.
void randomPick(PageList *plist) 
{
    if (plist == NULL || plist->head == NULL) 
    {
        printf("[ERROR][RandomPick] plist or plist->head is NULL.\n");
        exit(EXIT_FAILURE);
    }
    
    // 1) Count how many pages are in use
    int inUseCount = 0;
    Page* cur = plist->head;
    while (cur) 
    {
        if (cur->process_id != -1) 
        {
            inUseCount++;
        }
        cur = cur->next;
    }
    
    if (inUseCount == 0) 
    {
        // No pages in use => nothing to evict
        printf("[WARNING][RandomPick] No pages in use, cannot evict.\n");
        return;
    }
    
    // 2) Gather all in-use pages into an array
    Page** inUsePages = (Page**)malloc(inUseCount * sizeof(Page*));
    cur = plist->head;
    int idx = 0;
    while (cur) 
    {
        if (cur->process_id != -1) 
        {
            inUsePages[idx++] = cur;
        }
        cur = cur->next;
    }
    
    // 3) Pick a random index in [0, inUseCount-1]
    int r = rand() % inUseCount;
    Page* victim = inUsePages[r];
    
    printf("[DEBUG] Random Pick evicting Page %d from Process %d.\n",
           victim->page_num, victim->process_id);
    
    // 4) Free that page
    victim->process_id = -1;
    victim->page_num = -1;
    victim->brought_time = 0.0f;
    victim->last_referenced = 0.0f;
    victim->count = 0;
    
    free(inUsePages);
}

// uses randomPick(...) whenever a page fault occurs and no free page is available.

void randomPickSimulation(Process processes[], int numProcesses, PageList *plist, int* swaps, float* hit_ratio)
{
    if (plist == NULL || plist->head == NULL || processes == NULL) 
    {
        printf("[ERROR][RandomPickSim] Null pointers passed.\n");
        exit(EXIT_FAILURE);
    }

    int swap_count = 0;   // track # swaps
    int hit_count = 0;
    int miss_count = 0;
    int track_idx = 0;    // how many processes we have activated

    // Create a queue for all processes- sorted by arrival_time.
    Queue* readyQueue = createQueue(numProcesses);
    for(int i = 0; i < numProcesses; i++)
    {
        enqueue(readyQueue, processes[i]);
    }

    // Loop over time from 0 up to TOTAL_DURATION seconds
    for(int t = 0; t < TOTAL_DURATION; t++)
    {
        // If the queue is empty => all processes are done
        if(readyQueue->size == 0)
        {
            printf("[DEBUG][RandomPickSim] All processes finished. Breaking...\n");
            break;
        }

        // admit new processes whose arrival_time <= t
        while(track_idx < numProcesses 
           && readyQueue->processes[(readyQueue->front + track_idx) % readyQueue->capacity].arrival_time <= t)
        {
            int curr_idx = (readyQueue->front + track_idx) % readyQueue->capacity;
            Process* curr_proc = &readyQueue->processes[curr_idx];
            
            // If we have >=4 free pages, allocate the first page
            if(checkForFreePages(plist, 4))
            {
                printf("[DEBUG][RandomPickSim] Found >=4 free pages at time %d.\n", t);
                Page* p = getFreePage(plist);
                if(p)
                {
                    p->process_id = curr_proc->id;
                    p->page_num = curr_proc->starting_page_num;
                    p->brought_time = (float)t;
                    p->last_referenced = (float)t;
                    p->count = 1;  // used once
                    swap_count++;
                }
                track_idx++;
            }
            else
            {
                // Not enough free pages => can't admit this process yet
                break;
            }
        }

        // Within each second, 10 "mini-steps" for page references (100 ms each).
        for(int k = 0; k < 10; k++)
        {
            float currentSubTime = t + k * 0.1f;  //0.0, 0.1, 0.2, ...

            // For each admitted process
            for(int l = 0; l < track_idx; l++)
            {
                int idx = (readyQueue->front + l) % readyQueue->capacity;
                Process* proc = &readyQueue->processes[idx];

                // If the process is still in the queue and has service_time left
                if(proc->service_time > 0 && ifExistsInQueue(readyQueue, proc->id))
                {
                    // Generate the next page reference
                    proc->starting_page_num = getNextPage(proc->starting_page_num, proc->num_pages);

                    // Check if that page is in memory
                    if(ifExistsInMemory(plist, proc->id, proc->starting_page_num))
                    {
                        // HIT
                        Page* pageHit = getPageByID(plist, proc->id, proc->starting_page_num);
                        if(pageHit == NULL)
                        {
                            // Should not happen if ifExistsInMemory returned 1
                            printf("[ERROR][RandomPickSim] Page expected but not found.\n");
                            continue;
                        }
                        pageHit->count++;
                        pageHit->last_referenced = currentSubTime; 
                        
                        hit_count++;
                    }
                    else
                    {
                        // MISS => need a free page
                        Page* free_page = getFreePage(plist);
                        if(free_page == NULL)
                        {
                            // Memory is full => randomPick eviction
                            printf("[DEBUG][RandomPickSim] No free page at time %.1f => randomPick.\n", currentSubTime);
                            displayPages(plist);  
                            
                            randomPick(plist);
                            
                            printf("[DEBUG][RandomPickSim] After randomPick:\n");
                            displayPages(plist); 

                            // Now get a free page again
                            free_page = getFreePage(plist);
                            if(!free_page)
                            {
                                printf("[WARNING][RandomPickSim] Still no free page after randomPick.\n");
                                continue; // skip this reference
                            }
                        }
                        // Place the newly referenced page
                        free_page->process_id = proc->id;
                        free_page->page_num = proc->starting_page_num;
                        free_page->brought_time = currentSubTime;
                        free_page->last_referenced = currentSubTime;
                        free_page->count = 1; 
                        swap_count++;
                        miss_count++;
                    }
                }
            }
        }

        // Execute the processes for 1 second, decrementing service_time if their page is loaded
        for(int m = 0; m < track_idx; m++)
        {
            int curr_idx = (readyQueue->front + m) % readyQueue->capacity;
            Process* curr_proc = &readyQueue->processes[curr_idx];
            
            if(ifExistsInQueue(readyQueue, curr_proc->id))
            {
                // Check if the process's current page is in memory
                int page_loaded = ifExistsInMemory(plist, curr_proc->id, curr_proc->starting_page_num);
                if(!page_loaded)
                {
                    // Attempt to load it
                    Page* free_page = getFreePage(plist);
                    if(!free_page)
                    {
                        // No free page => stall
                        printf("[DEBUG][RandomPickSim] No free page to execute process %d => stall.\n", curr_proc->id);
                        continue;
                    }
                    else
                    {
                        free_page->process_id = curr_proc->id;
                        free_page->page_num = curr_proc->starting_page_num;
                        free_page->brought_time = (float)t;
                        free_page->last_referenced = (float)t;
                        free_page->count = 1;
                        swap_count++;
                        page_loaded = 1;
                    }
                }
                // If loaded, decrement service_time
                if(page_loaded && curr_proc->service_time > 0)
                {
                    curr_proc->service_time -= 1;
                }
                
                // If the process is finished
                if(curr_proc->service_time <= 0)
                {
                    // Do we need to check if the process is in queue????
                    printf("[DEBUG][RandomPickSim] Process %d finished, freeing memory.\n", curr_proc->id);
                    int done_pid = curr_proc->id;
                    removeFromQueue(readyQueue, curr_proc);
                    freeMemory(plist, done_pid);
                }
            }
        }

        // print memory map every 10 seconds
        if(t % 10 == 0)
        {
            printf("[DEBUG][RandomPickSim] Memory state at time %d sec:\n", t);
            printMemoryMap(plist);
        }
        usleep(900); // ~0.0009 seconds, just to slow the loop for readability
    }

    printf("[DEBUG][RandomPickSim] Final memory state:\n");
    printMemoryMap(plist); 

    // Compute stats
    *swaps = swap_count;
    int totalReferences = hit_count + miss_count;
    if (totalReferences > 0)
    {
        *hit_ratio = (float)hit_count / (float)totalReferences;
    }
    else
    {
        *hit_ratio = 0.0f;
    }
    printf("[DEBUG] Total number of sucessful swaps for Random Pick: %d, hit ratio: %.2f\n", *swaps, *hit_ratio);

    
    freeQueue(readyQueue);
}
