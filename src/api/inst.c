#include "inst.h"
#include "stack.h"
#include "macros.h"

#include <stdio.h>
#include <stdlib.h>

Instruction program[] = {    
    INST_PUSH(4),
    INST_PUSH(3),
    INST_OR(),

    INST_PRNT(),
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

void ClearStack() {
    if (stackSize == 0) return;

    int tempSize = stackSize; // need to make a temp copy because stackSize changes as we iterate
    for ( int i = 0; i < tempSize; i++ ) {
        Pop();
    } 
}

void PrintStack() {
    printf("--- Stack Start ---\n");
    for ( int i = stackSize-1; i >= 0; i-- ) {
        printf("%d\n", stack[i]);
    }
    printf("--- Stack End   ---\n");
}

void RunInstructions() {
    for (unsigned int i = 0; i < PROGRAM_SIZE; i++) {
        Instruction inst = program[i];
        switch ( inst.operation ) {
        case OP_MUL: {
            int a = Pop();
            int b = Pop();
            Push(a * b);
            break;
        }
        case OP_ANDB: {
            int b = Pop();
            int a = Pop();
            Push(a & b);
            break;
        }
        case OP_XORB: {
            int b = Pop();
            int a = Pop();
            Push(a ^ b);
            break;
        }
        case OP_NOTB: {
            int a = Pop();
            Push(~a);
            break;
        }
        case OP_ORB: {
            int b = Pop();
            int a = Pop();
            Push(a | b);
            break;
        }
        case OP_NEG: {
            int val = Pop();
            val *= -1;
            Push(val);
            break;
        }
        case OP_PUSH:
            Push(inst.data.value);
            break;
        case OP_POP: {
            int a = Pop();
            printf("Popped: %d\n", a);
            break;
        }
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
            Move(inst.data.registers.src, inst.data.registers.dest);
            break;
        }
        case OP_SUB: {
            int b = Pop();
            int a = Pop();
            Push(a - b);
            break;
        }
        case OP_CLR:
            ClearStack();
            break;
        case OP_SIZE:
            Push(stackSize);
            break;
        case OP_PRNT:
            PrintStack();
            break;
        }
    }
}