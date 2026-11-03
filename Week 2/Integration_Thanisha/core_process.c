
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
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>

#define REQUEST_QUEUE "/Yenkbachind_request"
#define RESPONSE_QUEUE "/Yenkbachind_response"
#define MAX_MSG_SIZE 256
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
static void send_log_message(int log_fd, const char *message)
{
    char buffer[MAX_MSG_SIZE];
    size_t length;
    size_t sent = 0;

    if (log_fd < 0 || message == NULL)
        return;

    snprintf(buffer, sizeof(buffer), "%s\n", message);
    length = strlen(buffer);

    while (sent < length)
    {
        ssize_t result = write(log_fd, buffer + sent, length - sent);

        if (result < 0)
        {
            if (errno == EINTR)
                continue;

            break;
        }

        sent += (size_t)result;
    }
}
int main(void)
{
    int channels[COMPONENT_COUNT][2];
    int worker_sockets[COMPONENT_COUNT];
    pid_t worker_pids[COMPONENT_COUNT];

    int log_pipe[2] = {-1, -1};
    pid_t logger_pid = -1;

    mqd_t request_queue = (mqd_t)-1;
    mqd_t response_queue = (mqd_t)-1;

    struct mq_attr queue_attributes;

    char message[MAX_MSG_SIZE + 1];
    char logger_fd[20];

    int i;
    int exit_status = EXIT_FAILURE;

    signal(SIGPIPE, SIG_IGN);

    /* Initialize worker descriptors */
    for (i = 0; i < COMPONENT_COUNT; i++)
    {
        channels[i][0] = -1;
        channels[i][1] = -1;
        worker_sockets[i] = -1;
        worker_pids[i] = -1;
    }

    /* Create socketpairs for four workers */
    for (i = 0; i < COMPONENT_COUNT; i++)
    {
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, channels[i]) < 0)
        {
            perror("socketpair");
            goto cleanup;
        }
    }

    /* Create CPU, Memory, Stack and Queue workers */
    for (i = 0; i < COMPONENT_COUNT; i++)
    {
        pid_t pid = fork();

        if (pid < 0)
        {
            perror("fork");
            goto cleanup;
        }

        if (pid == 0)
        {
            int j;
            int child_socket = channels[i][1];

            for (j = 0; j < COMPONENT_COUNT; j++)
            {
                if (channels[j][0] >= 0)
                    close(channels[j][0]);

                if (j != i && channels[j][1] >= 0)
                    close(channels[j][1]);
            }

            if (i == COMPONENT_CPU)
                cpu_worker(child_socket);

            if (i == COMPONENT_MEMORY)
                memory_worker(child_socket);

            if (i == COMPONENT_STACK)
                stack_worker(child_socket);

            queue_worker(child_socket);
        }

        worker_pids[i] = pid;
        worker_sockets[i] = channels[i][0];

        close(channels[i][1]);
        channels[i][1] = -1;
    }

    /* Create Core-to-Logger pipe */
    if (pipe(log_pipe) < 0)
    {
        perror("pipe");
        goto cleanup;
    }

    /* Start Logger process */
    logger_pid = fork();

    if (logger_pid < 0)
    {
        perror("Logger fork");
        goto cleanup;
    }

    if (logger_pid == 0)
    {
        close(log_pipe[1]);

        for (i = 0; i < COMPONENT_COUNT; i++)
        {
            if (worker_sockets[i] >= 0)
                close(worker_sockets[i]);
        }

        snprintf(logger_fd, sizeof(logger_fd), "%d", log_pipe[0]);

        execl("./logger", "logger", logger_fd, (char *)NULL);

        perror("Logger execution failed");
        _exit(EXIT_FAILURE);
    }

    close(log_pipe[0]);
    log_pipe[0] = -1;

    /* Configure POSIX Message Queues */
    memset(&queue_attributes, 0, sizeof(queue_attributes));
    queue_attributes.mq_maxmsg = 10;
    queue_attributes.mq_msgsize = MAX_MSG_SIZE;

    mq_unlink(REQUEST_QUEUE);
    mq_unlink(RESPONSE_QUEUE);

    request_queue = mq_open(
        REQUEST_QUEUE, O_CREAT | O_RDONLY, 0666, &queue_attributes
    );

    if (request_queue == (mqd_t)-1)
    {
        perror("Request queue creation failed");
        goto cleanup;
    }

    response_queue = mq_open(
        RESPONSE_QUEUE, O_CREAT | O_WRONLY, 0666, &queue_attributes
    );

    if (response_queue == (mqd_t)-1)
    {
        perror("Response queue creation failed");
        goto cleanup;
    }

    puts("Core Process Started");
    puts("All worker processes are running.");
    puts("POSIX Message Queues are ready.");
    puts("Logger is connected through a pipe.");
    puts("Ready");
    fflush(stdout);

    send_log_message(log_pipe[1], "[INFO] Core Process Started");

    /* Receive commands from UI */
    while (1)
    {
        CommandMessage command = {{0}};
        ResponseMessage response = {{0}};

        char operation[32];
        int values[2] = {0, 0};
        int arguments;
        int component = -1;

        ssize_t received = mq_receive(
            request_queue, message, MAX_MSG_SIZE, NULL
        );

        if (received < 0)
        {
            if (errno == EINTR)
                continue;

            perror("Message receive failed");
            break;
        }

        message[received] = '\0';

        if (strcmp(message, "exit") == 0 ||
            strcmp(message, "EXIT") == 0)
        {
            send_log_message(log_pipe[1],
                             "[INFO] UI requested shutdown");

            puts("Shutdown command received.");
            exit_status = EXIT_SUCCESS;
            break;
        }

        memcpy(command.text, message, sizeof(command.text) - 1);
            command.text[sizeof(command.text) - 1] = '\0';

        arguments = parse_command(
            command.text, operation, sizeof(operation), values
        );

        if (arguments < 0)
        {
            snprintf(response.text, sizeof(response.text),
                     "Invalid command");
        }
        else
        {
            if (strcmp(operation, "LOAD") == 0 ||
                strcmp(operation, "ADD") == 0 ||
                strcmp(operation, "SUB") == 0 ||
                strcmp(operation, "MUL") == 0 ||
                strcmp(operation, "DIV") == 0 ||
                strcmp(operation, "HALT") == 0)
            {
                component = COMPONENT_CPU;
            }
            else if (strcmp(operation, "STORE") == 0 ||
                     strcmp(operation, "READ") == 0)
            {
                component = COMPONENT_MEMORY;
            }
            else if (strcmp(operation, "PUSH") == 0 ||
                     strcmp(operation, "POP") == 0 ||
                     strcmp(operation, "PEEK") == 0)
            {
                component = COMPONENT_STACK;
            }
            else if (strcmp(operation, "ENQUEUE") == 0 ||
                     strcmp(operation, "DEQUEUE") == 0 ||
                     strcmp(operation, "QPEEK") == 0)
            {
                component = COMPONENT_QUEUE;
            }

            if (component < 0)
            {
                snprintf(response.text, sizeof(response.text),
                         "Unknown command");
            }
            else
            {
                if (!write_full(worker_sockets[component],
                                &command, sizeof(command)) ||
                    read_full(worker_sockets[component],
                              &response, sizeof(response)) != 1)
                {
                    memset(&response, 0, sizeof(response));

                    snprintf(response.text, sizeof(response.text),
                             "Operation failed");
                }
            }
        }

        /* Send response to UI */
        if (mq_send(response_queue, response.text,
                    strlen(response.text) + 1, 0) < 0)
        {
            perror("Message send failed");
            break;
        }

        send_log_message(log_pipe[1], message);
        send_log_message(log_pipe[1], response.text);

        printf("Command: %s\n", message);
        printf("Response: %s\n", response.text);
    }

    if (exit_status == EXIT_FAILURE)
        send_log_message(log_pipe[1], "[ERROR] Core stopped unexpectedly");

cleanup:

    puts("Stopping simulator...");

    if (request_queue != (mqd_t)-1)
        mq_close(request_queue);

    if (response_queue != (mqd_t)-1)
        mq_close(response_queue);

    mq_unlink(REQUEST_QUEUE);
    mq_unlink(RESPONSE_QUEUE);

    stop_workers(worker_sockets, worker_pids);

    if (log_pipe[1] >= 0)
    {
        send_log_message(log_pipe[1], "EXIT");
        close(log_pipe[1]);
        log_pipe[1] = -1;
    }

    if (log_pipe[0] >= 0)
        close(log_pipe[0]);

    if (logger_pid > 0)
        waitpid(logger_pid, NULL, 0);

    puts("Simulator stopped.");

    return exit_status;
}
