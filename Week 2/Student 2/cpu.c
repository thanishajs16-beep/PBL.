#include <stdio.h>
#include <string.h>

typedef struct {
    int accumulator;
    int program_counter;
    int running;
} CPU;

void execute_instruction(CPU *cpu, char instruction[])
{
    char opcode[10];
    int value;

    sscanf(instruction, "%s %d", opcode, &value);

    if (strcmp(opcode, "LOAD") == 0) {
        cpu->accumulator = value;
        printf("ACC = %d\n", cpu->accumulator);
    }
    else if (strcmp(opcode, "ADD") == 0) {
        cpu->accumulator += value;
        printf("ACC = %d\n", cpu->accumulator);
    }
    else if (strcmp(opcode, "SUB") == 0) {
        cpu->accumulator -= value;
        printf("ACC = %d\n", cpu->accumulator);
    }
    else if (strcmp(opcode, "MUL") == 0) {
        cpu->accumulator *= value;
        printf("ACC = %d\n", cpu->accumulator);
    }
    else if (strcmp(opcode, "DIV") == 0) {
        if (value == 0) {
            printf("Error: Cannot divide by zero\n");
        }
        else {
            cpu->accumulator /= value;
            printf("ACC = %d\n", cpu->accumulator);
        }
    }
    else if (strcmp(opcode, "HALT") == 0) {
        cpu->running = 0;
        printf("CPU stopped\n");
    }
    else {
        printf("Invalid instruction\n");
    }

    cpu->program_counter++;
}