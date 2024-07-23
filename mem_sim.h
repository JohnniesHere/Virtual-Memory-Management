#ifndef MEM_SIM_H
#define MEM_SIM_H

#define PAGE_SIZE 8
#define NUM_OF_PAGES 25
#define MEMORY_SIZE 40
#define SWAP_SIZE 200

// Structs ----------------------------------------------------------------------
typedef struct {
     int V; // valid
     int D; // dirty
     int P; // permission
     int frame_swap; // the number of a frame/swap if in case it is page-mapped
} page_descriptor;

struct sim_database
{
    page_descriptor page_table[NUM_OF_PAGES]; //pointer to page table
    int swapfile_fd; //swap file fd
    int program_fd; //executable file fd
    char main_memory[MEMORY_SIZE];
    int text_size;
    int data_size;
    int bss_heap_stack_size;
};


// Function signatures ---------------------------------------------------------
struct sim_database* init_system(char exe_file_name[], char swap_file_name[], int text_size, int data_size, int bss_heap_stack_size);
char load(struct sim_database* mem_sim, int address);
void store(struct sim_database* mem_sim, int address, char value);
void print_memory(struct sim_database* mem_sim);
void print_swap(struct sim_database* mem_sim);
void print_page_table(struct sim_database* mem_sim);
void clear_system(struct sim_database* mem_sim);


#endif // MEM_SIM_H