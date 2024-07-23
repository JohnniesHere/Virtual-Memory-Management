# Memory Paging Simulation

Authorize: Jonathan Elgarisi

## Memory Paging Simulation Program
This project simulates a processor's access to memory using a paging mechanism. It involves the implementation of functions to handle memory management, page faults, and page swapping.

## Features
- Simulate memory paging mechanism with a page table.
- Manage main memory and swap file.
- Handle page faults and page replacements.
- Perform memory load and store operations.
- Print memory, swap, and page table states.

## How to Run
To compile the program, use the following command:

```bash
gcc -Wall mem_sim.c main.c -o memory_simulator
```

Alternatively, you can use the provided `run_me.sh` script to compile and run the program:

```bash
./run_me.sh
```

## How to Use
After successfully compiling the program, you can run it using the following command:

```bash
./memory_simulator
```

The program will simulate the paging mechanism and provide the following functions for memory operations:

### Initialization
Initialize the system with the executable and swap file names, and sizes for text, data, and BSS/heap/stack sections:

```c
struct sim_database* init_system(char exe_file_name[], char swap_file_name[], int text_size, int data_size, int bss_heap_stack_size);
```

## Functions

### Load
Load a value from a specific address:

```c
char load(struct sim_database* mem_sim, int address);
```

### Store
Store a value at a specific address:

```c
void store(struct sim_database* mem_sim, int address, char value);
```

### Print Memory
Print the content of the main memory:

```c
void print_memory(struct sim_database* mem_sim);
```

### Print Swap
Print the content of the swap file:

```c
void print_swap(struct sim_database* mem_sim);
```

### Print Page Table
Print the page table:

```c
void print_page_table(struct sim_database* mem_sim);
```

### Clear System
Clear the system by closing files and freeing allocated memory:

```c
void clear_system(struct sim_database* mem_sim);
```

## Input
The program takes the following inputs during initialization:
- `exe_file_name`: Name of the executable file containing the program. This file is expected to contain the binary code of the program, which will be loaded into memory.
- `swap_file_name`: Name of the swap file. This file will be used to store pages that are swapped out of main memory.
- `text_size`: Size of the text section in bytes. This is the size of the compiled code section of the program.
- `data_size`: Size of the data section in bytes. This includes initialized global and static variables.
- `bss_heap_stack_size`: Combined size of the BSS, heap, and stack sections in bytes. The BSS section contains uninitialized global and static variables, while the heap and stack are used for dynamic memory allocation and function call management, respectively.

## Output
The program outputs the results of memory operations and prints the states of the main memory, swap file, and page table. It provides error messages if any issues occur.

## Files
- `mem_sim.c`: The main code containing the implementation of the memory paging simulation.
- `mem_sim.h`: The header file with the definitions and function signatures.
- `exec_file`: A file simulating an executable file for testing purposes.
- `main.c`: Contains comprehensive tests for the memory paging simulation.
- `run_me.sh`: A script to compile and run the program.
- `README.txt`: The text file you are currently reading.

## License
MIT - https://choosealicense.com/licenses/mit/

By following this README, you can compile and run the memory paging simulation program, which demonstrates how a processor manages memory using a paging mechanism, handles page faults, and swaps pages between main memory and a swap file.