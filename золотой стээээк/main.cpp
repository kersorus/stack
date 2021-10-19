#include <stdio.h>
#include <stdlib.h>
#include "stack.h"
#include "hash.h"

int main()
{
    stack_t *stack = stk_ator();
//    stack_t stack = {};
    stk_ctor(stack, int, 8, print_data_byte);
//    stk_ctor(&stack, int, 5, print_data_int);
    int val = 4;
    int str = 1;
    stk_push(stack, &val);
//    *(char *)(stack.data) = 0;
    val--;
//    stack.size = 10;
//    stack.esize = 0;
//    stack.create.func = 0;
    stk_dump(stack, stdout);
    stk_pop(stack, &str);
    printf("%d\n", str);
    stk_dtor(stack);
//    stk_dtor(&stack);

    return 0;
}
