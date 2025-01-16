#include "machine.h"

#include <stdio.h>
#include <malloc.h>


Machine* NewMachineWithInstructions(Instruction* ins) {
    Machine* machine = (Machine*)malloc(sizeof(Machine));
    machine->stackSize = 0;
    machine->program = ins;
    return machine;
}