#include <stdio.h>

#define QUEUE_SIZE 100

int queue[QUEUE_SIZE];
int front = 0;
int rear = -1;
int count = 0;

void enqueue(int value)
{
    if (count == QUEUE_SIZE) {
        printf("Error: Queue overflow\n");
        return;
    }

    rear = (rear + 1) % QUEUE_SIZE;
    queue[rear] = value;
    count++;

    printf("Enqueued %d\n", value);
}

int dequeue()
{
    int value;

    if (count == 0) {
        printf("Error: Queue underflow\n");
        return -1;
    }

    value = queue[front];
    front = (front + 1) % QUEUE_SIZE;
    count--;

    return value;
}

int queue_peek()
{
    if (count == 0) {
        printf("Queue is empty\n");
        return -1;
    }

    return queue[front];
}