#include <stdio.h>
#include <string.h>

typedef struct {
    int accumulator;
    int program_counter;
    int running;
} CPU;

/* CPU */
void execute_instruction(CPU *cpu, char instruction[]);

/* Memory */
void memory_write(int address, int value);
int memory_read(int address);

/* Stack */
void push(int value);
int pop(void);
int peek(void);

/* Queue */
void enqueue(int value);
int dequeue(void);
int queue_peek(void);

int main()
{
    CPU cpu = {0, 0, 1};

    char instruction[100];
    char operation[20];
    int value;
    int address;
    int result;

    while (cpu.running)
    {
        printf("\nEnter instruction: ");
        fgets(instruction, sizeof(instruction), stdin);

        instruction[strcspn(instruction, "\n")] = '\0';

        sscanf(instruction, "%s", operation);

        if (strcmp(operation, "STORE") == 0)
        {
            sscanf(instruction, "%s %d %d", operation, &address, &value);
            memory_write(address, value);
        }
        else if (strcmp(operation, "READ") == 0)
        {
            sscanf(instruction, "%s %d", operation, &address);
            result = memory_read(address);

            if (result != -1)
                printf("Memory[%d] = %d\n", address, result);
        }
        else if (strcmp(operation, "PUSH") == 0)
        {
            sscanf(instruction, "%s %d", operation, &value);
            push(value);
        }
        else if (strcmp(operation, "POP") == 0)
        {
            result = pop();

            if (result != -1)
                printf("Popped %d\n", result);
        }
        else if (strcmp(operation, "PEEK") == 0)
        {
            result = peek();

            if (result != -1)
                printf("Top = %d\n", result);
        }
        else if (strcmp(operation, "ENQUEUE") == 0)
        {
            sscanf(instruction, "%s %d", operation, &value);
            enqueue(value);
        }
        else if (strcmp(operation, "DEQUEUE") == 0)
        {
            result = dequeue();

            if (result != -1)
                printf("Dequeued %d\n", result);
        }
        else if (strcmp(operation, "QPEEK") == 0)
        {
            result = queue_peek();

            if (result != -1)
                printf("Front = %d\n", result);
        }
        else
        {
            execute_instruction(&cpu, instruction);
        }
    }

    printf("\nFinal ACC = %d\n", cpu.accumulator);
    printf("Final PC = %d\n", cpu.program_counter);

    return 0;
}