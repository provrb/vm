#include "inst.h"
#include "macros.h"

#include <stdio.h>
#include <stdlib.h>

void Move(Machine* machine, int src, int dest) {
    if ( machine->stackSize <= src || dest < 0 || src < 0 || machine->stackSize <= dest ) {
        fprintf(stderr, "Trying to move value from %d to %d. Out-of-bounds.\n", src, dest);
        exit(1);
    }
    machine->stack[dest] = machine->stack[src];
}

void Push(Machine* machine, int value) {
    if ( machine->stackSize >= STACK_CAPACITY ) {
        fprintf(stderr, "Stack overflow when trying to push value to stack: %d\n", value);
        exit(1);
    }

    machine->stack[machine->stackSize++] = value;
}

int Pop(Machine* machine) {
    if ( machine->stackSize <= 0 ) {
        fprintf(stderr, "Stack underflow when trying to pop from stack.\n");
        exit(1);
    }

    return machine->stack[--machine->stackSize];
}

void ClearStack(Machine* machine) {
    if (machine->stackSize == 0) return;

    int tempSize = machine->stackSize; // need to make a temp copy because stackSize changes as we iterate
    for ( int i = 0; i < tempSize; i++ ) {
        Pop(machine);
    } 
}

void PrintStack(Machine* machine) {
    printf("--- Stack Start ---\n");
    for ( int i = machine->stackSize-1; i >= 0; i-- ) {
        printf("%d\n", machine->stack[i]);
    }
    printf("--- Stack End   ---\n");
}

void RunInstructions(Machine* machine) {
    Instruction* instructions = (Instruction*)machine->program;
    if ( !instructions ) {
        fprintf(stderr, "Panic: Instructions invalid!\n");
        return;
    }

    for (unsigned int i = 0; i < machine->programSize; i++) { 
        Instruction inst = instructions[i];

        switch ( inst.operation ) {
        case OP_NOP:
            break;
        case OP_SHL: {
            int val = Pop(machine);
            Push(machine, val << inst.data.value);
            break;
        }
        case OP_SHR: {
            int val = Pop(machine);
            Push(machine, val >> inst.data.value);
            break;
        }
        case OP_SWAP: {
            int first = Pop(machine);
            int second = Pop(machine);
            Push(machine, second);
            Push(machine, first);
            break;
        }
        case OP_MUL: {
            int a = Pop(machine);
            int b = Pop(machine);
            Push(machine, a * b);
            break;
        }
        case OP_DUP:
            Push(machine, machine->stack[machine->stackSize-1]);
            break;
        case OP_ANDB: {
            int b = Pop(machine);
            int a = Pop(machine);
            Push(machine, a & b);
            break;
        }
        case OP_XORB: {
            int b = Pop(machine);
            int a = Pop(machine);
            Push(machine, a ^ b);
            break;
        }
        case OP_NOTB: {
            int a = Pop(machine);
            Push(machine, ~a);
            break;
        }
        case OP_ORB: {
            int b = Pop(machine);
            int a = Pop(machine);
            Push(machine, a | b);
            break;
        }
        case OP_NEG: {
            int val = Pop(machine);
            val *= -1;
            Push(machine, val);
            break;
        }
        case OP_PUSH:
            Push(machine, inst.data.value);
            break;
        case OP_POP:
            Pop(machine);
            break;
        case OP_ADD: {
            int a = Pop(machine);
            int b = Pop(machine);
            Push(machine, a + b);
            break;
        }
        case OP_DIV: {  
            int b = Pop(machine);
            int a = Pop(machine); // you put the bigger number first in div
            
            if ( b == 0 ) {
                fprintf(stderr, "Divide by zero error. (%d / %d)\n", a, b);
                exit(1);
            }

            Push(machine, a / b);
            break;
        }
        case OP_MOD: {
            int b = Pop(machine);
            int a = Pop(machine);
            Push(machine, a % b);
            break;
        }
        case OP_MOV:
            Move(machine, inst.data.registers.src, inst.data.registers.dest);
            break;
        case OP_SUB: {
            int b = Pop(machine);
            int a = Pop(machine);
            Push(machine, a - b);
            break;
        }
        case OP_CLR:
            ClearStack(machine);
            break;
        case OP_SIZE:
            Push(machine, machine->stackSize);
            break;
        case OP_PRNT:
            PrintStack(machine);
            break;
        }
    }
}