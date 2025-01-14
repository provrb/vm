#include <stdio.h>
#include <stdlib.h>

#define STACK_CAPACITY 1024
#define PROGRAM_SIZE sizeof(program) / sizeof(program[0])

#define INST_PUSH(a) {.operation = OP_PUSH, .value = a}
#define INST_POP() {.operation = OP_POP}
#define INST_ADD() {.operation = OP_ADD}
#define INST_DIV() {.operation = OP_DIV}
#define INST_PRNT() {.operation = OP_PRNT}
#define INST_MOD() {.operation = OP_MOD}
#define INST_MOV(a, b) {.operation = OP_MOV, .src = a, .dest = b}
#define INST_SUB() {.operation = OP_SUB}

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
    OP_MOV,

    OP_PRNT,
} Opcode;

typedef struct {
    Opcode operation;
    int value;
    int src; // for mov
    int dest; // for mov
} Instruction;

// Instruction program[] = {
//     INST_PUSH(13),
//     INST_PUSH(12),
//     INST_ADD(), // 25
//     INST_PUSH(5), 
//     INST_ADD(), // 30
//     INST_PUSH(10), // 30 and 10 on stack
//     INST_DIV(), // 30 divided by 10 is 3
//     INST_PUSH(3), 
//     INST_MOD(), // 3 % 3 == 0
//     INST_PUSH(10),
//     INST_PUSH(11),
//     INST_PUSH(12),
//     INST_MOV(3, 2),

//     INST_PRNT(),
// };

Instruction program[] = {
    INST_PUSH(3),
    INST_PUSH(0),
    INST_DIV(),
};

void Move(int src, int dest) {
    if ( stackSize <= src || dest < 0 || src < 0 || stackSize <= dest ) {
        fprintf(stderr, "Trying to move value from %d to %d. Out-of-bounds.\n", src, dest);
        exit(1);
    }
    stack[dest] = stack[src];
}

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
            
            if ( b == 0 ) {
                fprintf(stderr, "Divide by zero error. (%d / %d)\n", a, b);
                exit(1);
            }

            Push(a / b);
            break;
        }
        case OP_MOD: {
            int b = Pop();
            int a = Pop();
            Push(a % b);
            break;
        }
        case OP_MOV: {
            Move(inst.src, inst.dest);
            break;
        }
        case OP_SUB: {

        }
        case OP_PRNT:
            PrintStack();
            break;
        }
    }

    return 0;
}