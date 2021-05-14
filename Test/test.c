#include "stack.h"

int main(){

    int *numero = malloc(sizeof(int));
    *numero = 45;

    struct Stack *stack = createStack(10);

    push(stack, numero);

    int* ptr = pop(stack);
    printf("Direccion: %d\n", ptr);
    printf("Objecto: %d\n", *ptr);

    

    return 0;
}