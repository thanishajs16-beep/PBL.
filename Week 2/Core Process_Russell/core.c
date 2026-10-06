#include <stdio.h>
#include <string.h>

typedef struct {
    int accumulator;
    int program_counter;
    int running;
} CPU;

void execute_instruction(CPU *cpu, char instruction[]);

void memory_write(int address, int value);
int memory_read(int address);

void push(int value);
int pop(void);
int peek(void);

void enqueue(int value);
int dequeue(void);
int queue_peek(void);

void process_command(CPU *cpu, char instruction[], char response[], int response_size)
{
    char operation[20];
    int value;
    int address;
    int result;

    sscanf(instruction, "%s", operation);

    if (strcmp(operation, "STORE") == 0)
    {
        sscanf(instruction, "%s %d %d", operation, &address, &value);
        memory_write(address, value);

        snprintf(response, response_size,
                 "Memory[%d] = %d", address, value);
    }
    else if (strcmp(operation, "READ") == 0)
    {
        sscanf(instruction, "%s %d", operation, &address);
        result = memory_read(address);

        if (result != -1)
        {
            snprintf(response, response_size,
                     "Memory[%d] = %d", address, result);
        }
        else
        {
            snprintf(response, response_size,
                     "Invalid memory address");
        }
    }
    else if (strcmp(operation, "PUSH") == 0)
    {
        sscanf(instruction, "%s %d", operation, &value);
        push(value);

        snprintf(response, response_size,
                 "Pushed %d", value);
    }
    else if (strcmp(operation, "POP") == 0)
    {
        result = pop();

        if (result != -1)
        {
            snprintf(response, response_size,
                     "Popped %d", result);
        }
        else
        {
            snprintf(response, response_size,
                     "Stack is empty");
        }
    }
    else if (strcmp(operation, "PEEK") == 0)
    {
        result = peek();

        if (result != -1)
        {
            snprintf(response, response_size,
                     "Top = %d", result);
        }
        else
        {
            snprintf(response, response_size,
                     "Stack is empty");
        }
    }
    else if (strcmp(operation, "ENQUEUE") == 0)
    {
        sscanf(instruction, "%s %d", operation, &value);
        enqueue(value);

        snprintf(response, response_size,
                 "Enqueued %d", value);
    }
    else if (strcmp(operation, "DEQUEUE") == 0)
    {
        result = dequeue();

        if (result != -1)
        {
            snprintf(response, response_size,
                     "Dequeued %d", result);
        }
        else
        {
            snprintf(response, response_size,
                     "Queue is empty");
        }
    }
    else if (strcmp(operation, "QPEEK") == 0)
    {
        result = queue_peek();

        if (result != -1)
        {
            snprintf(response, response_size,
                     "Front = %d", result);
        }
        else
        {
            snprintf(response, response_size,
                     "Queue is empty");
        }
    }
    else
    {
        execute_instruction(cpu, instruction);

        snprintf(response, response_size,
                 "ACC = %d, PC = %d",
                 cpu->accumulator,
                 cpu->program_counter);
    }
}
