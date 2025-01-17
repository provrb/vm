#include "api/macros.h"
#include "api/machine.h"
#include "api/inst.h"

#include <stdlib.h>

int main() {
    Instruction p[] = {
        INST_PUSH(2),
        INST_PUSH(1),
        INST_ADD(),
        INST_PUSH(2),
        INST_JGE(3),

        INST_PRNT(),
    };

    Machine* machine = malloc(sizeof(Machine));
    machine->stackSize = 0;
    machine->ip = 0;
    machine->program = p;
    machine->programSize = sizeof(p) / sizeof(Instruction);

    RunInstructions(machine);

    free(machine);
    return 0;
}