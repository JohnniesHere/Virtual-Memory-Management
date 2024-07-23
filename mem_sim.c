#include "mem_sim.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

// Declare global static variables ----------------------------------------------
static int NEXT_FRAME_TO_SWAP = 0;
static int NEXT_SWAP_LOCATION = 0;

// Function to reset the static variables ---------------------------------------
void reset_swap_variables() {
    NEXT_FRAME_TO_SWAP = 0;
    NEXT_SWAP_LOCATION = 0;
}

// Forward declarations of static helper functions ----------------------------
static int handle_page_fault(struct sim_database *mem_sim, int page_number);
static int find_free_frame(struct sim_database *mem_sim);
static bool is_frame_free(struct sim_database *mem_sim, int frame);
static int swap_out_page(struct sim_database *mem_sim);
static void copy_page_from_exe(struct sim_database *mem_sim, int page_number, int frame);
static void bring_page_from_swap(struct sim_database *mem_sim, int page_number, int frame);

//Helper Functions ------------------------------------------------------------
// Helper function to handle page faults --------------------------------------
static int handle_page_fault(struct sim_database *mem_sim, int page_number) {
    int frame = find_free_frame(mem_sim);
    if (frame == -1) {
        frame = swap_out_page(mem_sim);
        if (frame == -1) {
            fprintf(stderr, "Error: No page can be swapped out\n");
            return -1;
        }
    }

     page_descriptor *page = &mem_sim->page_table[page_number];
    if (page_number * PAGE_SIZE < mem_sim->text_size + mem_sim->data_size) {
        copy_page_from_exe(mem_sim, page_number, frame);
    } else if (page->frame_swap != -1) {
        bring_page_from_swap(mem_sim, page_number, frame);
    } else {
        memset(&mem_sim->main_memory[frame * PAGE_SIZE], '0', PAGE_SIZE);
    }

    page->V = 1;
    page->frame_swap = frame;
    return frame;
}

// Helper function to find a free frame ----------------------------------------
static int find_free_frame(struct sim_database *mem_sim) {
    for (int i = 0; i < MEMORY_SIZE / PAGE_SIZE; i++) {
        if (is_frame_free(mem_sim, i)) {
            return i;
        }
    }
    return -1;
}

// Helper function to check if a frame is free ---------------------------------
static bool is_frame_free(struct sim_database *mem_sim, int frame) {
    for (int i = 0; i < NUM_OF_PAGES; i++) {
        if (mem_sim->page_table[i].V && mem_sim->page_table[i].frame_swap == frame) {
            return false;
        }
    }
    return true;
}

// Helper function to swap out a page ----------------------------------------
static int swap_out_page(struct sim_database *mem_sim) {
    int frames_checked = 0;
    int total_frames = MEMORY_SIZE / PAGE_SIZE;

    while (frames_checked < total_frames) {
        int frame_to_check = NEXT_FRAME_TO_SWAP;

        for (int i = 0; i < NUM_OF_PAGES; i++) {
             page_descriptor *page = &mem_sim->page_table[i];
            if (page->V && page->frame_swap == frame_to_check) {
                // Write page to swap if dirty
                if (page->D) {
                    lseek(mem_sim->swapfile_fd, NEXT_SWAP_LOCATION * PAGE_SIZE, SEEK_SET);
                    write(mem_sim->swapfile_fd, &mem_sim->main_memory[frame_to_check * PAGE_SIZE], PAGE_SIZE);
                    page->frame_swap = NEXT_SWAP_LOCATION; // Store the swap location only if dirty
                    NEXT_SWAP_LOCATION = (NEXT_SWAP_LOCATION + 1) % (SWAP_SIZE / PAGE_SIZE); // Update NEXT_SWAP_LOCATION only if a page is written to swap
                } else {
                    page->frame_swap = -1; // Indicate that the page was not written to swap
                }

                // Mark frame as free
                memset(&mem_sim->main_memory[frame_to_check * PAGE_SIZE], '0', PAGE_SIZE);
                page->V = 0;

                // Update next_frame_to_swap
                NEXT_FRAME_TO_SWAP = (NEXT_FRAME_TO_SWAP + 1) % total_frames;
                printf("Swapped out page %d from frame %d to swap location %d\n", i, frame_to_check, page->frame_swap);
                return frame_to_check;
            }
        }

        // If we didn't find a page in this frame, move to the next one
        NEXT_FRAME_TO_SWAP = (NEXT_FRAME_TO_SWAP + 1) % total_frames;
        frames_checked++;
    }

    // If we've checked all frames and found nothing to swap, return -1
    return -1;
}

// Helper function to copy a page from the executable -----------------------
static void copy_page_from_exe(struct sim_database *mem_sim, int page_number, int frame) {
    lseek(mem_sim->program_fd, page_number * PAGE_SIZE, SEEK_SET);
    read(mem_sim->program_fd, &mem_sim->main_memory[frame * PAGE_SIZE], PAGE_SIZE);
}

// Helper function to bring a page in from swap ------------------------------
static void bring_page_from_swap(struct sim_database *mem_sim, int page_number, int frame) {
     page_descriptor *page = &mem_sim->page_table[page_number];
    lseek(mem_sim->swapfile_fd, page->frame_swap * PAGE_SIZE, SEEK_SET);
    read(mem_sim->swapfile_fd, &mem_sim->main_memory[frame * PAGE_SIZE], PAGE_SIZE);
    page->frame_swap = -1; // Reset swap location after loading
}

// Main Functions -------------------------------------------------------------
// Initializes the system -------------------------------------------------------
struct sim_database* init_system(char exe_file_name[], char swap_file_name[], int text_size, int data_size, int bss_heap_stack_size) {
    struct sim_database *sim_db = (struct sim_database *)malloc(sizeof(struct sim_database));
    if (!sim_db) {
        perror("Failed to allocate memory for sim_database");
        return NULL;
    }

    sim_db->program_fd = open(exe_file_name, O_RDONLY);
    if (sim_db->program_fd < 0) {
        perror("Failed to open executable file");
        free(sim_db);
        return NULL;
    }

    sim_db->swapfile_fd = open(swap_file_name, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (sim_db->swapfile_fd < 0) {
        perror("Failed to open or create swap file");
        close(sim_db->program_fd);
        free(sim_db);
        return NULL;
    }

    // Initialize main memory with '0'
    memset(sim_db->main_memory, '0', MEMORY_SIZE);

    // Initialize swap file with '0'
    char buffer[SWAP_SIZE];
    memset(buffer, '0', SWAP_SIZE);
    write(sim_db->swapfile_fd, buffer, SWAP_SIZE);
    lseek(sim_db->swapfile_fd, 0, SEEK_SET);

    for (int i = 0; i < NUM_OF_PAGES; i++) {
        sim_db->page_table[i].V = 0;
        sim_db->page_table[i].D = 0;
        sim_db->page_table[i].P = (i < text_size / PAGE_SIZE) ? 1 : 0;
        sim_db->page_table[i].frame_swap = -1;
    }

    sim_db->text_size = text_size;
    sim_db->data_size = data_size;
    sim_db->bss_heap_stack_size = bss_heap_stack_size;

    return sim_db;
}

// Tries to access a certain address for reading ------------------------------
char load(struct sim_database* mem_sim, int address) {
    int page_number = address / PAGE_SIZE;
    int offset = address % PAGE_SIZE;

    printf("Loading address %d (page %d, offset %d)\n", address, page_number, offset);

    if (page_number >= NUM_OF_PAGES) {
        fprintf(stderr, "Error: Invalid address\n");
        return '\0';
    }

     page_descriptor *page = &mem_sim->page_table[page_number];

    printf("Page state before load: V=%d, D=%d, P=%d, frame_swap=%d\n",
           page->V, page->D, page->P, page->frame_swap);

    if (!page->V) {
        int frame = handle_page_fault(mem_sim, page_number);
        if (frame == -1) {
            fprintf(stderr, "Error: Failed to handle page fault\n");
            return '\0';
        }
    }

    int physical_address = page->frame_swap * PAGE_SIZE + offset;
    char value = mem_sim->main_memory[physical_address];

    printf("Page state after load: V=%d, D=%d, P=%d, frame_swap=%d\n",
           page->V, page->D, page->P, page->frame_swap);
    printf("Loaded value: '%c'\n", value);

    return value;
}

// Tries to access a certain address for writing -------------------------------
void store(struct sim_database* mem_sim, int address, char value) {
    int page_number = address / PAGE_SIZE;
    int offset = address % PAGE_SIZE;

    printf("Storing value '%c' at address %d (page %d, offset %d)\n", value, address, page_number, offset);

    if (page_number >= NUM_OF_PAGES) {
        fprintf(stderr, "Error: Invalid address\n");
        return;
    }

     page_descriptor *page = &mem_sim->page_table[page_number];

    printf("Page state before store: V=%d, D=%d, P=%d, frame_swap=%d\n",
           page->V, page->D, page->P, page->frame_swap);

    if (page->P) {
        fprintf(stderr, "Error: Write access denied to read-only page\n");
        return;
    }

    if (!page->V) {
        int frame = handle_page_fault(mem_sim, page_number);
        if (frame == -1) {
            fprintf(stderr, "Error: Failed to handle page fault\n");
            return;
        }
    }

    int physical_address = page->frame_swap * PAGE_SIZE + offset;
    mem_sim->main_memory[physical_address] = value;
    page->D = 1;

    printf("Page state after store: V=%d, D=%d, P=%d, frame_swap=%d\n",
           page->V, page->D, page->P, page->frame_swap);
    printf("Stored value: '%c'\n", value);
}

//Print Functions --------------------------------------------------------------
// Prints the content of the main memory -------------------------------------
void print_memory(struct sim_database* mem_sim) {
    printf("\nPhysical memory\n");
    for(int i = 0; i < MEMORY_SIZE; i++) {
        printf("[%c]\n", mem_sim->main_memory[i]);
    }
}

// Prints the content of the SWAP file ----------------------------------------
void print_swap(struct sim_database* mem_sim) {
    char str[PAGE_SIZE];
    printf("\nSwap memory\n");
    lseek(mem_sim->swapfile_fd, 0, SEEK_SET); // go to the start of the file
    while(read(mem_sim->swapfile_fd, str, PAGE_SIZE) == PAGE_SIZE) {
        for(int i = 0; i < PAGE_SIZE; i++) {
            printf("[%c]\t", str[i]);
        }
        printf("\n");
    }
}

// Prints the page table -------------------------------------------------------
void print_page_table(struct sim_database* mem_sim) {
    printf("\nPage table\n");
    printf("Valid\tDirty\tPermission\tFrame_swap\n");
    for(int i = 0; i < NUM_OF_PAGES; i++) {
        printf("[%d]\t[%d]\t[%d]\t[%d]\n", mem_sim->page_table[i].V, mem_sim->page_table[i].D, mem_sim->page_table[i].P, mem_sim->page_table[i].frame_swap);
    }
}

// Closes open files and frees dynamically allocated memory ----------------
void clear_system(struct sim_database* mem_sim) {
    close(mem_sim->program_fd);
    close(mem_sim->swapfile_fd);
    free(mem_sim);
    reset_swap_variables();
}