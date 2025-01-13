#include <stdio.h>
#include <stdlib.h>

#define STACK_CAPACITY 1024
#define PROGRAM_SIZE sizeof(program) / sizeof(program[0])

#define INST_PUSH(a) {.operation = OP_PUSH, .value = a}
#define INST_POP() {.operation = OP_POP}
#define INST_ADD() {.operation = OP_ADD}
#define INST_DIV() {.operation = OP_DIV}
#define INST_PRNT() {.operation = OP_PRNT}

int stack[STACK_CAPACITY];
int stackSize = 0;

typedef enum {
    OP_PUSH,
    OP_POP,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_PRNT,
} Opcode;

typedef struct {
    Opcode operation;
    int value;
} Instruction;

Instruction program[] = {
    INST_PUSH(13),
    INST_PUSH(12),
    INST_ADD(), // 25
    INST_PUSH(5), 
    INST_ADD(), // 30
    INST_PUSH(10), // 30 and 10 on stack
    INST_DIV(), // 30 divided by 10

    
    INST_PRNT(),
};

void Push(int value) {
    if ( stackSize >= STACK_CAPACITY ) {
        fprintf(stderr, "Stack overflow when trying to push value to stack: %d\n", value);
        exit(1);
    }

    stack[stackSize++] = value;
}

int Pop() {
    if ( stackSize <= 0 ) {
        fprintf(stderr, "Stack underflow when trying to pop from stack.\n");
        exit(1);
    }

    return stack[--stackSize];
}

void PrintStack() {
    printf("--- Stack Start ---\n");
    for ( int i = 0; i < stackSize; i++ ) {
        printf("%d\n", stack[i]);
    }
    printf("--- Stack End   ---\n");
}

int main() {
    for (int i = 0; i < PROGRAM_SIZE; i++) {
        Instruction inst = program[i];
        switch ( inst.operation ) {
        case OP_PUSH:
            Push(inst.value);
            break;
        case OP_POP:
            Pop();
            break;
        case OP_ADD: {
            int a = Pop();
            int b = Pop();
            Push(a + b);
            break;
        }
        case OP_DIV: {
            int b = Pop();
            int a = Pop(); // you put the bigger number first in div
            Push(a / b);
            break;
        }
        case OP_MOD: {
            int b = Pop();
            int a = Pop();
            Push(a % b);
            break;
        }
        case OP_PRNT:
            PrintStack();
            break;
        }
    }

    return 0;
}