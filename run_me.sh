#!/bin/bash

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Compile the program
echo -e "${GREEN}Compiling the program...${NC}"
gcc -Wall main.c mem_sim.c -o memory_simulator

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo -e "${GREEN}Compilation successful!${NC}"
    
    # Run the program
    echo -e "${GREEN}Running the program...${NC}"
    ./memory_simulator
    
    # Check if the program ran successfully
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}Program completed successfully!${NC}"
    else
        echo -e "${RED}Program encountered an error during execution.${NC}"
    fi
else
    echo -e "${RED}Compilation failed. Please check your code for errors.${NC}"
fi

# Clean up
echo -e "${GREEN}Cleaning up...${NC}"
rm -f memory_simulator
