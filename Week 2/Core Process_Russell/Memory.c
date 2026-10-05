#include <stdio.h>

#define MEMORY_SIZE 100

int memory[MEMORY_SIZE];

void memory_write(int address, int value)
{
    if (address < 0 || address >= MEMORY_SIZE) {
        printf("Error: Invalid memory address\n");
        return;
    }

    memory[address] = value;
    printf("Memory[%d] = %d\n", address, value);
}

int memory_read(int address)
{
    if (address < 0 || address >= MEMORY_SIZE) {
        printf("Error: Invalid memory address\n");
        return -1;
    }

    return memory[address];
}