#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INSTRUCTION_SIZE 256

typedef struct {
    int accumulator;
    int program_counter;
    int running;
} CPU;

static int parse_integer(const char *text, int *value)
{
    char *end = NULL;
    long parsed;

    errno = 0;
    parsed = strtol(text, &end, 10);

    if (text[0] == '\0' || end == text || *end != '\0' ||
        errno == ERANGE || parsed < INT_MIN || parsed > INT_MAX) {
        return 0;
    }

    *value = (int)parsed;
    return 1;
}

static void set_response(char *response, size_t response_size,
                         const char *message)
{
    if (response_size > 0) {
        snprintf(response, response_size, "%s", message);
    }
}

void execute_instruction(CPU *cpu, char instruction[],
                         char response[], size_t response_size)
{
    char buffer[INSTRUCTION_SIZE];
    char *tokens[3];
    char *save_pointer = NULL;
    char *token;
    int values[2] = {0, 0};
    int token_count = 0;
    int operand_count;
    long long result;
    size_t i;

    if (cpu == NULL || instruction == NULL || response == NULL ||
        response_size == 0) {
        return;
    }

    if (strlen(instruction) >= sizeof(buffer)) {
        set_response(response, response_size, "Instruction is too long");
        return;
    }

    strcpy(buffer, instruction);
    token = strtok_r(buffer, " ,\t\r\n", &save_pointer);

    while (token != NULL && token_count < 3) {
        tokens[token_count++] = token;
        token = strtok_r(NULL, " ,\t\r\n", &save_pointer);
    }

    if (token != NULL || token_count == 0) {
        set_response(response, response_size, "Invalid instruction");
        return;
    }

    for (i = 0; tokens[0][i] != '\0'; i++) {
        tokens[0][i] = (char)toupper((unsigned char)tokens[0][i]);
    }

    if (!cpu->running) {
        set_response(response, response_size, "CPU is halted");
        return;
    }

    if (strcmp(tokens[0], "HALT") == 0) {
        if (token_count != 1) {
            set_response(response, response_size, "Invalid instruction");
            return;
        }

        cpu->running = 0;
        cpu->program_counter++;
        snprintf(response, response_size, "Halted at PC = %d",
                 cpu->program_counter);
        return;
    }

    if (strcmp(tokens[0], "LOAD") == 0) {
        if (token_count != 2 || !parse_integer(tokens[1], &values[0])) {
            set_response(response, response_size, "Invalid instruction");
            return;
        }

        cpu->accumulator = values[0];
        cpu->program_counter++;
        snprintf(response, response_size, "ACC = %d, PC = %d",
                 cpu->accumulator, cpu->program_counter);
        return;
    }

    if (strcmp(tokens[0], "ADD") != 0 &&
        strcmp(tokens[0], "SUB") != 0 &&
        strcmp(tokens[0], "MUL") != 0 &&
        strcmp(tokens[0], "DIV") != 0) {
        set_response(response, response_size, "Invalid instruction");
        return;
    }

    operand_count = token_count - 1;
    if (operand_count < 1 || operand_count > 2 ||
        !parse_integer(tokens[1], &values[0]) ||
        (operand_count == 2 && !parse_integer(tokens[2], &values[1]))) {
        set_response(response, response_size, "Invalid instruction");
        return;
    }

    if (operand_count == 1) {
        values[1] = values[0];
        values[0] = cpu->accumulator;
    }

    if (strcmp(tokens[0], "ADD") == 0) {
        result = (long long)values[0] + values[1];
    } else if (strcmp(tokens[0], "SUB") == 0) {
        result = (long long)values[0] - values[1];
    } else if (strcmp(tokens[0], "MUL") == 0) {
        result = (long long)values[0] * values[1];
    } else {
        if (values[1] == 0) {
            set_response(response, response_size, "Division by zero");
            return;
        }
        if (values[0] == INT_MIN && values[1] == -1) {
            set_response(response, response_size, "Integer overflow");
            return;
        }
        result = values[0] / values[1];
    }

    if (result < INT_MIN || result > INT_MAX) {
        set_response(response, response_size, "Integer overflow");
        return;
    }

    cpu->accumulator = (int)result;
    cpu->program_counter++;
    snprintf(response, response_size, "ACC = %d, PC = %d",
             cpu->accumulator, cpu->program_counter);
}
