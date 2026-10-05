#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "ipc.h"

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

int main(void)
{
    mqd_t message_queue;
    struct mq_attr attributes;
    char message[MAX_MESSAGE_SIZE + 1];
    ssize_t bytes_received;

    attributes.mq_flags = 0;
    attributes.mq_maxmsg = MAX_MESSAGES;
    attributes.mq_msgsize = MAX_MESSAGE_SIZE;
    attributes.mq_curmsgs = 0;

    message_queue = mq_open(
        QUEUE_NAME,
        O_CREAT | O_RDONLY,
        0666,
        &attributes
    );

    if (message_queue == (mqd_t)-1)
    {
        perror("Error opening POSIX message queue");
        return 1;
    }

    printf("Logger Process Started\n");
    printf("Waiting for messages from Core Process...\n");

    write_log("[INFO] Logger Process Started");

    while (1)
    {
        bytes_received = mq_receive(
            message_queue,
            message,
            MAX_MESSAGE_SIZE,
            NULL
        );

        if (bytes_received == -1)
        {
            perror("Error receiving message");
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

    mq_close(message_queue);
    mq_unlink(QUEUE_NAME);

    printf("Logger Process Stopped\n");

    return 0;
}