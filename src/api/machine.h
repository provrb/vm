#ifndef MACHINE_H
#define MACHINE_H

#include "macros.h"

typedef struct {
    int stackSize;
    int stack[STACK_CAPACITY];
    void* program; // this should be an array of Instruction
    int programSize;
} Machine;

Machine* NewWithInstructions(void* ins);
void Reset(Machine* m);

#endif