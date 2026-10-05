#include <stdio.h>

#define STACK_SIZE 100

int stack[STACK_SIZE];
int top = -1;

void push(int value)
{
    if (top == STACK_SIZE - 1) {
        printf("Error: Stack overflow\n");
        return;
    }

    stack[++top] = value;
    printf("Pushed %d\n", value);
}

int pop()
{
    if (top == -1) {
        printf("Error: Stack underflow\n");
        return -1;
    }

    return stack[top--];
}

int peek()
{
    if (top == -1) {
        printf("Stack is empty\n");
        return -1;
    }

    return stack[top];
}