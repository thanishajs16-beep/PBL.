#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define CORE_SOCKET_PATH "/tmp/pbl_core_socket"
#define COMMAND_SIZE 256
#define RESPONSE_SIZE 256

typedef struct {
    char text[COMMAND_SIZE];
} CommandMessage;

typedef struct {
    char text[RESPONSE_SIZE];
} ResponseMessage;

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

static void show_help(void)
{
    puts("CPU: LOAD n, ADD n [m], SUB n [m], MUL n [m], DIV n [m], HALT");
    puts("Memory: STORE address value, READ address");
    puts("Stack: PUSH n, POP, PEEK");
    puts("Queue: ENQUEUE n, DEQUEUE, QPEEK");
    puts("Other: HELP, EXIT, SHUTDOWN");
}

int main(void)
{
    int connection;
    struct sockaddr_un address;
    char input[COMMAND_SIZE];

    connection = socket(AF_UNIX, SOCK_STREAM, 0);
    if (connection < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }

    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    snprintf(address.sun_path, sizeof(address.sun_path), "%s",
             CORE_SOCKET_PATH);

    if (connect(connection, (struct sockaddr *)&address, sizeof(address)) < 0) {
        puts("Run ./core in another terminal first.");
        close(connection);
        return EXIT_FAILURE;
    }

    while (1) {
        CommandMessage command = {{0}};
        ResponseMessage response = {{0}};
        size_t length;

        printf("> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        length = strcspn(input, "\r\n");
        input[length] = '\0';

        if (input[0] == '\0') {
            continue;
        }

        if (strcmp(input, "EXIT") == 0 || strcmp(input, "exit") == 0) {
            break;
        }

        if (strcmp(input, "HELP") == 0 || strcmp(input, "help") == 0) {
            show_help();
            continue;
        }

        if (strlen(input) >= sizeof(command.text)) {
            puts("Command is too long");
            continue;
        }

        snprintf(command.text, sizeof(command.text), "%s", input);
        if (!write_full(connection, &command, sizeof(command)) ||
            read_full(connection, &response, sizeof(response)) != 1) {
            puts("Connection closed");
            break;
        }

        puts(response.text);

        if (strcmp(input, "SHUTDOWN") == 0 || strcmp(input, "shutdown") == 0) {
            break;
        }
    }

    close(connection);
    return EXIT_SUCCESS;
}
