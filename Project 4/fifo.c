// fifo.c
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <string.h>
#include "utils.h"
#include "helper.h"

//---------------------------------------------------------------------
// Helper: Convert a process ID to a process name (a single character).
// For example, process ID 1 => 'A', 2 => 'B', …, 27 => 'A', etc.
//---------------------------------------------------------------------
static char getProcessName(int id) {
    return 'A' + ((id - 1) % 26);
}

//---------------------------------------------------------------------
// Helper: Print the current memory map of 100 pages.
// Each page is represented by the process name (if in use) or a dot if free.
//---------------------------------------------------------------------
static void printMemoryMapCustom(PageList *plist) {
    char mem_map[TOTAL_PAGE_REFS + 1];
    int pos = 0;
    Page *current = plist->head;
    while (current != NULL && pos < TOTAL_PAGE_REFS) {
        if (current->process_id != -1)
            mem_map[pos] = getProcessName(current->process_id);
        else
            mem_map[pos] = '.';
        pos++;
        current = current->next;
    }
    mem_map[pos] = '\0';
    printf("Memory Map: %s\n", mem_map);
}

//---------------------------------------------------------------------
// FIFO Victim Selection Function
//
// Scans the physical memory (PageList) and selects the page with the 
// oldest brought_time (i.e. the one that was loaded earliest) for eviction.
//---------------------------------------------------------------------
Page* fifo(PageList *plist) {
    if (plist == NULL) {
        fprintf(stderr, "[FIFO] Error: PageList is NULL\n");
        return NULL;
    }
    
    Page *current = plist->head;
    Page *victim = NULL;
    float oldestTime = FLT_MAX;
    
    while (current != NULL) {
        // Only consider pages that are in use (process_id != -1)
        if (current->process_id != -1 && current->brought_time < oldestTime) {
            oldestTime = current->brought_time;
            victim = current;
        }
        current = current->next;
    }
    
    if (victim != NULL) {
        printf("[FIFO] Selected victim: Process %d, Page %d (brought_time = %.2f)\n", 
               victim->process_id, victim->page_num, victim->brought_time);
    }
    
    return victim;
}

//---------------------------------------------------------------------
// FIFO Simulation Function
//
// This function simulates the FIFO page replacement algorithm for the 
// entire workload. It goes through the list of generated processes (assumed
// to be sorted by arrival_time) and simulates each process’s execution.
// For each process that arrives before TOTAL_DURATION (60 sec) and for which 
// at least 4 free pages exist, the process is swapped in. Then, every 0.1 sec,
// the process makes a memory reference (using getNextPage) until its service time
// expires. Detailed records for each memory reference are printed.
// Additionally, whenever a process is swapped in or completes, a record is printed 
// along with the current memory map.
//---------------------------------------------------------------------
void fifoSimulation(Process processes[], int numProcesses, PageList *plist, int *swaps, float *hit_ratio) {
    float current_time = 0.0;
    int swapped_in = 0;
    int hits = 0, misses = 0;
    
    printf("[FIFO] Starting FIFO Simulation (1-minute run)...\n");
    
    // Loop through each process (processes are assumed to be sorted by arrival_time)
    for (int i = 0; i < numProcesses; i++) {
        // Only process jobs arriving within the 60-second simulation window
        if (processes[i].arrival_time > TOTAL_DURATION)
            break;
        
        // Advance simulation time to the process's arrival time if necessary
        if (processes[i].arrival_time > current_time)
            current_time = processes[i].arrival_time;
        
        // Check if there are at least 4 free pages available before swapping in
        if (!checkForFreePages(plist, 4)) {
            printf("[FIFO] Not enough free pages for Process %d at time %.2f\n",
                   processes[i].id, current_time);
            continue;
        }
        
        // Swap in the process: load its initial page (page 0)
        Page *freePage = getFreePage(plist);
        if (freePage != NULL) {
            freePage->process_id = processes[i].id;
            freePage->page_num = 0;
            freePage->brought_time = current_time;
            freePage->last_referenced = current_time;
            freePage->count = 1;
            swapped_in++;
            
            // Print process "Enter" record with memory map
            printf("\n[ENTER] Time: %.2f, Process %c, Size: %d pages, Service Time: %d sec\n", 
                   current_time, getProcessName(processes[i].id), processes[i].num_pages, processes[i].service_time);
            printMemoryMapCustom(plist);
        } else {
            printf("[FIFO] No free page available for Process %d at time %.2f\n",
                   processes[i].id, current_time);
            continue;
        }
        
        int current_page = 0;
        int num_references = processes[i].service_time * 10; // 10 references per second
        
        // Simulate each memory reference (every 0.1 sec)
        for (int ref = 0; ref < num_references; ref++) {
            int next_page = getNextPage(current_page, processes[i].num_pages);
            char evictedInfo[50] = "None";
            int pageInMemory = ifExistsInMemory(plist, processes[i].id, next_page);
            
            if (pageInMemory) {
                // Page hit
                hits++;
                Page *page = getPageByID(plist, processes[i].id, next_page);
                if (page != NULL) {
                    page->last_referenced = current_time;
                    page->count++;
                }
                printf("Time: %.2f, Process: %c, Page: %d, InMemory: YES, Evicted: %s\n", 
                       current_time, getProcessName(processes[i].id), next_page, evictedInfo);
            } else {
                // Page miss
                misses++;
                // Try to get a free page
                Page *freePg = getFreePage(plist);
                if (freePg != NULL) {
                    freePg->process_id = processes[i].id;
                    freePg->page_num = next_page;
                    freePg->brought_time = current_time;
                    freePg->last_referenced = current_time;
                    freePg->count = 1;
                    strcpy(evictedInfo, "None");
                    printf("Time: %.2f, Process: %c, Page: %d, InMemory: NO, Evicted: %s\n", 
                           current_time, getProcessName(processes[i].id), next_page, evictedInfo);
                } else {
                    // No free page available; use FIFO to select a victim
                    Page *victim = fifo(plist);
                    if (victim != NULL) {
                        snprintf(evictedInfo, sizeof(evictedInfo), "Process %c, Page %d", 
                                 getProcessName(victim->process_id), victim->page_num);
                        // Evict the victim and load the new page
                        victim->process_id = processes[i].id;
                        victim->page_num = next_page;
                        victim->brought_time = current_time;
                        victim->last_referenced = current_time;
                        victim->count = 1;
                    }
                    printf("Time: %.2f, Process: %c, Page: %d, InMemory: NO, Evicted: %s\n", 
                           current_time, getProcessName(processes[i].id), next_page, evictedInfo);
                }
            }
            
            current_page = next_page;
            current_time += 0.1;  // Advance simulation time by 0.1 seconds
            
            // (Optional) If you wish to stop the simulation at 60 seconds exactly:
            // if (current_time >= TOTAL_DURATION) break;
        }
        
        // Process finished its service (or simulation run time expired)
        printf("\n[EXIT] Time: %.2f, Process %c completed, Size: %d pages, Service Time: %d sec\n", 
               current_time, getProcessName(processes[i].id), processes[i].num_pages, processes[i].service_time);
        printMemoryMapCustom(plist);
        
        // Free the process's pages from memory
        freeMemory(plist, processes[i].id);
    }
    
    // Set output parameters for overall simulation statistics
    *swaps = swapped_in;
    int total_refs = hits + misses;
    *hit_ratio = (total_refs > 0) ? ((float)hits / total_refs) : 0.0;
    
    printf("\n[FIFO Simulation Complete] Processes Swapped-In: %d, Total Page References: %d, Hit Ratio: %.5f\n", 
           swapped_in, total_refs, *hit_ratio);
}

