#include "machine.h"

#include <malloc.h>
#include <memory.h>
#include <stdio.h>

Machine* NewWithInstructions(void* ins) {
    Machine* machine = (Machine*)malloc(sizeof(Machine));
    machine->stackSize = 0;
    machine->program = ins;
    return machine;
}

void Reset(Machine* m) {
    memset(m->stack, 0, sizeof m->stack);
    memset(m->program, 0, sizeof m->program);
    m->stackSize = 0;
    m->programSize = 0;
    m->ip = 0;
}