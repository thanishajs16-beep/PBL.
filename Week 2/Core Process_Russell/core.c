#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

#define CORE_SOCKET_PATH "/tmp/pbl_core_socket"
#define COMMAND_SIZE 256
#define RESPONSE_SIZE 256
#define COMPONENT_COUNT 4
#define COMPONENT_CAPACITY 100

typedef struct {
    char text[COMMAND_SIZE];
} CommandMessage;

typedef struct {
    char text[RESPONSE_SIZE];
} ResponseMessage;

typedef struct {
    int accumulator;
    int program_counter;
    int running;
} CPU;

void execute_instruction(CPU *cpu, char instruction[],
                         char response[], size_t response_size);
void memory_write(int address, int value);
int memory_read(int address);
void push(int value);
int pop(void);
int peek(void);
void enqueue(int value);
int dequeue(void);
int queue_peek(void);

typedef enum {
    COMPONENT_CPU = 0,
    COMPONENT_MEMORY,
    COMPONENT_STACK,
    COMPONENT_QUEUE
} Component;

static int read_full(int descriptor, void *buffer, size_t size)
{
    size_t received = 0;
    char *position = buffer;

    while (received < size) {
        ssize_t result = recv(descriptor, position + received,
                              size - received, 0);
        if (result == 0) {
            return 0;
        }
        if (result < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        received += (size_t)result;
    }

    return 1;
}

static int write_full(int descriptor, const void *buffer, size_t size)
{
    size_t sent = 0;
    const char *position = buffer;

    while (sent < size) {
        ssize_t result = send(descriptor, position + sent, size - sent,
                              MSG_NOSIGNAL);
        if (result < 0) {
            if (errno == EINTR) {
                continue;
            }
            return 0;
        }
        if (result == 0) {
            return 0;
        }
        sent += (size_t)result;
    }

    return 1;
}

static int parse_command(const char *text, char *operation,
                         size_t operation_size, int values[2])
{
    char buffer[COMMAND_SIZE];
    char *tokens[3];
    char *save_pointer = NULL;
    char *token;
    int count = 0;
    int i;

    if (text == NULL || operation == NULL || operation_size == 0 ||
        strlen(text) >= sizeof(buffer)) {
        return -1;
    }

    strcpy(buffer, text);
    token = strtok_r(buffer, " ,\t\r\n", &save_pointer);

    while (token != NULL && count < 3) {
        tokens[count++] = token;
        token = strtok_r(NULL, " ,\t\r\n", &save_pointer);
    }

    if (token != NULL || count == 0 ||
        strlen(tokens[0]) >= operation_size) {
        return -1;
    }

    strcpy(operation, tokens[0]);
    for (i = 0; operation[i] != '\0'; i++) {
        operation[i] = (char)toupper((unsigned char)operation[i]);
    }

    for (i = 1; i < count; i++) {
        char *end = NULL;
        long number;

        errno = 0;
        number = strtol(tokens[i], &end, 10);
        if (tokens[i][0] == '\0' || end == tokens[i] || *end != '\0' ||
            errno == ERANGE || number < INT_MIN || number > INT_MAX) {
            return -1;
        }
        values[i - 1] = (int)number;
    }

    return count - 1;
}

static void cpu_worker(int descriptor)
{
    CPU cpu = {0, 0, 1};
    CommandMessage command;
    ResponseMessage response;

    while (read_full(descriptor, &command, sizeof(command)) == 1) {
        if (strcmp(command.text, "__STOP__") == 0) {
            break;
        }

        memset(&response, 0, sizeof(response));
        execute_instruction(&cpu, command.text, response.text,
                            sizeof(response.text));
        if (!write_full(descriptor, &response, sizeof(response))) {
            break;
        }
    }

    close(descriptor);
    _exit(EXIT_SUCCESS);
}

static void memory_worker(int descriptor)
{
    CommandMessage command;
    ResponseMessage response;

    while (read_full(descriptor, &command, sizeof(command)) == 1) {
        char operation[32];
        int values[2] = {0, 0};
        int arguments;

        if (strcmp(command.text, "__STOP__") == 0) {
            break;
        }

        memset(&response, 0, sizeof(response));
        arguments = parse_command(command.text, operation,
                                  sizeof(operation), values);

        if (arguments == 2 && strcmp(operation, "STORE") == 0) {
            if (values[0] < 0 || values[0] >= COMPONENT_CAPACITY) {
                snprintf(response.text, sizeof(response.text),
                         "Invalid memory address");
            } else {
                memory_write(values[0], values[1]);
                snprintf(response.text, sizeof(response.text),
                         "Memory[%d] = %d", values[0], values[1]);
            }
        } else if (arguments == 1 && strcmp(operation, "READ") == 0) {
            if (values[0] < 0 || values[0] >= COMPONENT_CAPACITY) {
                snprintf(response.text, sizeof(response.text),
                         "Invalid memory address");
            } else {
                snprintf(response.text, sizeof(response.text),
                         "Memory[%d] = %d", values[0],
                         memory_read(values[0]));
            }
        } else {
            snprintf(response.text, sizeof(response.text),
                     "Invalid memory command");
        }

        if (!write_full(descriptor, &response, sizeof(response))) {
            break;
        }
    }

    close(descriptor);
    _exit(EXIT_SUCCESS);
}

static void stack_worker(int descriptor)
{
    CommandMessage command;
    ResponseMessage response;
    int item_count = 0;

    while (read_full(descriptor, &command, sizeof(command)) == 1) {
        char operation[32];
        int values[2] = {0, 0};
        int arguments;

        if (strcmp(command.text, "__STOP__") == 0) {
            break;
        }

        memset(&response, 0, sizeof(response));
        arguments = parse_command(command.text, operation,
                                  sizeof(operation), values);

        if (arguments == 1 && strcmp(operation, "PUSH") == 0) {
            if (item_count >= COMPONENT_CAPACITY) {
                snprintf(response.text, sizeof(response.text), "Stack is full");
            } else {
                push(values[0]);
                item_count++;
                snprintf(response.text, sizeof(response.text),
                         "Pushed %d", values[0]);
            }
        } else if (arguments == 0 && strcmp(operation, "POP") == 0) {
            if (item_count == 0) {
                snprintf(response.text, sizeof(response.text), "Stack is empty");
            } else {
                int value = pop();
                item_count--;
                snprintf(response.text, sizeof(response.text),
                         "Popped %d", value);
            }
        } else if (arguments == 0 && strcmp(operation, "PEEK") == 0) {
            if (item_count == 0) {
                snprintf(response.text, sizeof(response.text), "Stack is empty");
            } else {
                snprintf(response.text, sizeof(response.text),
                         "Top = %d", peek());
            }
        } else {
            snprintf(response.text, sizeof(response.text),
                     "Invalid stack command");
        }

        if (!write_full(descriptor, &response, sizeof(response))) {
            break;
        }
    }

    close(descriptor);
    _exit(EXIT_SUCCESS);
}

static void queue_worker(int descriptor)
{
    CommandMessage command;
    ResponseMessage response;
    int item_count = 0;

    while (read_full(descriptor, &command, sizeof(command)) == 1) {
        char operation[32];
        int values[2] = {0, 0};
        int arguments;

        if (strcmp(command.text, "__STOP__") == 0) {
            break;
        }

        memset(&response, 0, sizeof(response));
        arguments = parse_command(command.text, operation,
                                  sizeof(operation), values);

        if (arguments == 1 && strcmp(operation, "ENQUEUE") == 0) {
            if (item_count >= COMPONENT_CAPACITY) {
                snprintf(response.text, sizeof(response.text), "Queue is full");
            } else {
                enqueue(values[0]);
                item_count++;
                snprintf(response.text, sizeof(response.text),
                         "Enqueued %d", values[0]);
            }
        } else if (arguments == 0 && strcmp(operation, "DEQUEUE") == 0) {
            if (item_count == 0) {
                snprintf(response.text, sizeof(response.text), "Queue is empty");
            } else {
                int value = dequeue();
                item_count--;
                snprintf(response.text, sizeof(response.text),
                         "Dequeued %d", value);
            }
        } else if (arguments == 0 && strcmp(operation, "QPEEK") == 0) {
            if (item_count == 0) {
                snprintf(response.text, sizeof(response.text), "Queue is empty");
            } else {
                snprintf(response.text, sizeof(response.text),
                         "Front = %d", queue_peek());
            }
        } else {
            snprintf(response.text, sizeof(response.text),
                     "Invalid queue command");
        }

        if (!write_full(descriptor, &response, sizeof(response))) {
            break;
        }
    }

    close(descriptor);
    _exit(EXIT_SUCCESS);
}

static void stop_workers(int worker_sockets[COMPONENT_COUNT],
                         pid_t worker_pids[COMPONENT_COUNT])
{
    int i;
    CommandMessage stop_message = {{0}};

    snprintf(stop_message.text, sizeof(stop_message.text), "__STOP__");

    for (i = 0; i < COMPONENT_COUNT; i++) {
        if (worker_sockets[i] >= 0) {
            (void)write_full(worker_sockets[i], &stop_message,
                             sizeof(stop_message));
            close(worker_sockets[i]);
        }
    }

    for (i = 0; i < COMPONENT_COUNT; i++) {
        if (worker_pids[i] > 0) {
            (void)waitpid(worker_pids[i], NULL, 0);
        }
    }
}

int main(void)
{
    int channels[COMPONENT_COUNT][2];
    int worker_sockets[COMPONENT_COUNT];
    pid_t worker_pids[COMPONENT_COUNT];
    int listener = -1;
    int i;
    int shutting_down = 0;
    struct sockaddr_un address;

    signal(SIGPIPE, SIG_IGN);

    for (i = 0; i < COMPONENT_COUNT; i++) {
        channels[i][0] = -1;
        channels[i][1] = -1;
        worker_sockets[i] = -1;
        worker_pids[i] = -1;
    }

    for (i = 0; i < COMPONENT_COUNT; i++) {
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, channels[i]) < 0) {
            perror("socketpair");
            for (int j = 0; j < COMPONENT_COUNT; j++) {
                if (channels[j][0] >= 0) close(channels[j][0]);
                if (channels[j][1] >= 0) close(channels[j][1]);
            }
            return EXIT_FAILURE;
        }
    }

    for (i = 0; i < COMPONENT_COUNT; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            stop_workers(worker_sockets, worker_pids);
            for (int j = i; j < COMPONENT_COUNT; j++) {
                if (channels[j][0] >= 0) close(channels[j][0]);
                if (channels[j][1] >= 0) close(channels[j][1]);
            }
            return EXIT_FAILURE;
        }

        if (pid == 0) {
            int j;
            int child_socket = channels[i][1];

            for (j = 0; j < COMPONENT_COUNT; j++) {
                if (channels[j][0] >= 0) close(channels[j][0]);
                if (j != i && channels[j][1] >= 0) close(channels[j][1]);
            }

            if (i == COMPONENT_CPU) cpu_worker(child_socket);
            if (i == COMPONENT_MEMORY) memory_worker(child_socket);
            if (i == COMPONENT_STACK) stack_worker(child_socket);
            queue_worker(child_socket);
        }

        worker_pids[i] = pid;
        worker_sockets[i] = channels[i][0];
        close(channels[i][1]);
        channels[i][1] = -1;
    }

    listener = socket(AF_UNIX, SOCK_STREAM, 0);
    if (listener < 0) {
        perror("socket");
        stop_workers(worker_sockets, worker_pids);
        return EXIT_FAILURE;
    }

    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    snprintf(address.sun_path, sizeof(address.sun_path), "%s",
             CORE_SOCKET_PATH);
    unlink(CORE_SOCKET_PATH);

    if (bind(listener, (struct sockaddr *)&address, sizeof(address)) < 0 ||
        listen(listener, 5) < 0) {
        perror("socket setup");
        close(listener);
        unlink(CORE_SOCKET_PATH);
        stop_workers(worker_sockets, worker_pids);
        return EXIT_FAILURE;
    }

    puts("Ready");
    fflush(stdout);

    while (!shutting_down) {
        int client = accept(listener, NULL, NULL);

        if (client < 0) {
            if (errno == EINTR) continue;
            perror("accept");
            break;
        }

        for (;;) {
            CommandMessage command = {{0}};
            ResponseMessage response = {{0}};
            char operation[32];
            int values[2] = {0, 0};
            int arguments;
            int component = -1;

            if (read_full(client, &command, sizeof(command)) != 1) {
                break;
            }

            arguments = parse_command(command.text, operation,
                                      sizeof(operation), values);
            if (arguments < 0) {
                snprintf(response.text, sizeof(response.text), "Invalid command");
                (void)write_full(client, &response, sizeof(response));
                continue;
            }

            if (strcmp(operation, "SHUTDOWN") == 0 && arguments == 0) {
                snprintf(response.text, sizeof(response.text), "Closed");
                (void)write_full(client, &response, sizeof(response));
                shutting_down = 1;
                break;
            }

            if (strcmp(operation, "LOAD") == 0 ||
                strcmp(operation, "ADD") == 0 ||
                strcmp(operation, "SUB") == 0 ||
                strcmp(operation, "MUL") == 0 ||
                strcmp(operation, "DIV") == 0 ||
                strcmp(operation, "HALT") == 0) {
                component = COMPONENT_CPU;
            } else if (strcmp(operation, "STORE") == 0 ||
                       strcmp(operation, "READ") == 0) {
                component = COMPONENT_MEMORY;
            } else if (strcmp(operation, "PUSH") == 0 ||
                       strcmp(operation, "POP") == 0 ||
                       strcmp(operation, "PEEK") == 0) {
                component = COMPONENT_STACK;
            } else if (strcmp(operation, "ENQUEUE") == 0 ||
                       strcmp(operation, "DEQUEUE") == 0 ||
                       strcmp(operation, "QPEEK") == 0) {
                component = COMPONENT_QUEUE;
            }

            if (component < 0) {
                snprintf(response.text, sizeof(response.text),
                         "Unknown command. Enter HELP for commands.");
                (void)write_full(client, &response, sizeof(response));
                continue;
            }

            if (!write_full(worker_sockets[component], &command,
                            sizeof(command)) ||
                read_full(worker_sockets[component], &response,
                          sizeof(response)) != 1) {
                memset(&response, 0, sizeof(response));
                snprintf(response.text, sizeof(response.text),
                         "Operation failed");
            }

            if (!write_full(client, &response, sizeof(response))) {
                break;
            }
        }

        close(client);
    }

    close(listener);
    unlink(CORE_SOCKET_PATH);
    stop_workers(worker_sockets, worker_pids);
    return EXIT_SUCCESS;
}
