#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>

#define REQUEST_QUEUE  "/Yenkbachind_request"
#define RESPONSE_QUEUE "/Yenkbachind_response"
#define LOGGER_QUEUE   "/expression_logger"

#define MAX_MSG_SIZE 256

typedef struct {
    int accumulator;
    int program_counter;
    int running;
} CPU;

void process_command(CPU *cpu, char instruction[],
                     char response[], int response_size);

int main(void)
{
    mqd_t request_queue;
    mqd_t response_queue;
    mqd_t logger_queue;

    struct mq_attr attr;

    char message[MAX_MSG_SIZE];
    char response[MAX_MSG_SIZE];
    char log_message[MAX_MSG_SIZE];

    CPU cpu = {0, 0, 1};

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    /* Create request queue */
    request_queue = mq_open(
        REQUEST_QUEUE,
        O_CREAT | O_RDONLY,
        0666,
        &attr
    );

    if (request_queue == (mqd_t)-1)
    {
        perror("Core: Cannot create request queue");
        return 1;
    }

    /* Create response queue */
    response_queue = mq_open(
        RESPONSE_QUEUE,
        O_CREAT | O_WRONLY,
        0666,
        &attr
    );

    if (response_queue == (mqd_t)-1)
    {
        perror("Core: Cannot create response queue");
        mq_close(request_queue);
        mq_unlink(REQUEST_QUEUE);
        return 1;
    }

    /* Connect to Logger */
    logger_queue = mq_open(
        LOGGER_QUEUE,
        O_WRONLY
    );

    if (logger_queue == (mqd_t)-1)
    {
        perror("Core: Cannot connect to Logger");
        mq_close(request_queue);
        mq_close(response_queue);
        mq_unlink(REQUEST_QUEUE);
        mq_unlink(RESPONSE_QUEUE);
        return 1;
    }

    printf("Core: Waiting for UI...\n");

    while (cpu.running)
    {
        ssize_t bytes;

        /* Receive command from UI */
        bytes = mq_receive(
            request_queue,
            message,
            MAX_MSG_SIZE,
            NULL
        );

        if (bytes == -1)
        {
            perror("Core: Failed to receive message");
            break;
        }

        message[bytes] = '\0';

        /* Handle exit command */
        if (strcmp(message, "exit") == 0)
        {
            snprintf(
                response,
                MAX_MSG_SIZE,
                "Core shutting down"
            );

            /* Send response to UI */
            mq_send(
                response_queue,
                response,
                strlen(response) + 1,
                0
            );

            /* Tell Logger to stop */
            mq_send(
                logger_queue,
                "EXIT",
                strlen("EXIT") + 1,
                0
            );

            break;
        }

        /* Execute command */
        process_command(
            &cpu,
            message,
            response,
            MAX_MSG_SIZE
        );

        /* Send result back to UI */
        if (mq_send(
                response_queue,
                response,
                strlen(response) + 1,
                0) == -1)
        {
            perror("Core: Failed to send response");
            break;
        }

        /* Log errors with [ERROR] and normal results with [INFO] */
        if (strcmp(response, "Invalid memory address") == 0 ||
            strcmp(response, "Stack is empty") == 0 ||
            strcmp(response, "Queue is empty") == 0)
        {
            snprintf(
                log_message,
                MAX_MSG_SIZE,
                "[ERROR] Command: %s | Result: %s",
                message,
                response
            );
        }
        else
        {
            snprintf(
                log_message,
                MAX_MSG_SIZE,
                "[INFO] Command: %s | Result: %s",
                message,
                response
            );
        }

        /* Send log message to Logger */
        if (mq_send(
                logger_queue,
                log_message,
                strlen(log_message) + 1,
                0) == -1)
        {
            perror("Core: Failed to send log message");
        }
    }

    mq_close(request_queue);
    mq_close(response_queue);
    mq_close(logger_queue);

    mq_unlink(REQUEST_QUEUE);
    mq_unlink(RESPONSE_QUEUE);

    printf("Core: Shutdown complete.\n");

    return 0;
}