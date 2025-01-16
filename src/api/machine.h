#ifndef MACHINE_H
#define MACHINE_H

#include "macros.h"
#include "inst.h"

typedef struct {
    int stackSize;
    int stack[STACK_CAPACITY];
    Instruction* program;
} Machine;

Machine* NewMachineWithInstructions(Instruction* ins);

#endif