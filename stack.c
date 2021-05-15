#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

struct Stack {
    int top;
    int capacidad;
    pthread_t **ptr;
};

struct Stack* createStack(unsigned capacidad)
{
    struct Stack* stack = (struct Stack*)malloc(sizeof(struct Stack));
    stack->capacidad = capacidad;
    stack->top = -1;
    stack->ptr = calloc(sizeof(pthread_t), capacidad);
    return stack;
}

// Stack is full when top is equal to the last index
int isFull(struct Stack* stack)
{
    return stack->top == stack->capacidad - 1;
}
 
// Stack is empty when top is equal to -1
int isEmpty(struct Stack* stack)
{
    return stack->top == -1;
}
 
// Function to add an item to stack.  It increases top by 1
void push(struct Stack* stack, pthread_t *item)
{
    if (isFull(stack))
        return;
    stack->ptr[++stack->top] = item;
    printf("%d pushed to stack\n", item);
}
 
// Function to remove an item from stack.  It decreases top by 1
pthread_t* pop(struct Stack* stack)
{
    if (isEmpty(stack))
        return NULL;
    return stack->ptr[stack->top--];
}
 
// Function to return the top from stack without removing it
pthread_t* peek(struct Stack* stack)
{
    if (isEmpty(stack))
        return NULL;
    return stack->ptr[stack->top];
}
 
