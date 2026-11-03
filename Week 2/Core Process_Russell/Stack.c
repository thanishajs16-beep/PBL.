#define STACK_SIZE 100

static int stack[STACK_SIZE];
static int top = -1;

void push(int value)
{
    if (top < STACK_SIZE - 1) {
        stack[++top] = value;
    }
}

int pop(void)
{
    if (top < 0) {
        return -1;
    }

    return stack[top--];
}

int peek(void)
{
    if (top < 0) {
        return -1;
    }

    return stack[top];
}
