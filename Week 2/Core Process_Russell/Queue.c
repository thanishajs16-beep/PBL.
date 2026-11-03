#define QUEUE_SIZE 100

static int queue[QUEUE_SIZE];
static int front = 0;
static int rear = -1;
static int item_count = 0;

void enqueue(int value)
{
    if (item_count >= QUEUE_SIZE) {
        return;
    }

    rear = (rear + 1) % QUEUE_SIZE;
    queue[rear] = value;
    item_count++;
}

int dequeue(void)
{
    int value;

    if (item_count == 0) {
        return -1;
    }

    value = queue[front];
    front = (front + 1) % QUEUE_SIZE;
    item_count--;

    if (item_count == 0) {
        rear = front - 1;
        if (rear < 0) {
            rear = QUEUE_SIZE - 1;
        }
    }

    return value;
}

int queue_peek(void)
{
    if (item_count == 0) {
        return -1;
    }

    return queue[front];
}
