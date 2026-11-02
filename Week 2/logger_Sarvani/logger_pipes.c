
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ipc.h"

#define BUFFER_SIZE (MAX_MESSAGE_SIZE + 1)

void write_log(const char *message)
{
    FILE *log_file;

    log_file = fopen("simulator.log", "a");

    if (log_file == NULL)
    {
        perror("Error opening simulator.log");
        return;
    }

    fprintf(log_file, "%s\n", message);

    fclose(log_file);
}

int main(int argc, char *argv[])
{
    int pipe_fd;
    char message[BUFFER_SIZE];
    ssize_t bytes_received;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <pipe_read_fd>\n", argv[0]);
        return 1;
    }

    pipe_fd = atoi(argv[1]);

    printf("Logger Process Started\n");
    printf("Waiting for messages from Core Process...\n");

    write_log("[INFO] Logger Process Started");

    while (1)
    {
        bytes_received = read(pipe_fd, message, MAX_MESSAGE_SIZE);

        if (bytes_received == -1)
        {
            perror("Error receiving message");
            break;
        }

        if (bytes_received == 0)
        {
            break;
        }

        message[bytes_received] = '\0';

        if (strcmp(message, "EXIT") == 0)
        {
            write_log("[INFO] Logger Process Stopped");
            break;
        }

        printf("Log message received: %s\n", message);

        write_log(message);
    }

    close(pipe_fd);

    printf("Logger Process Stopped\n");

    return 0;
}
