#include "api/macros.h"
#include "api/machine.h"
#include "api/inst.h"

#include <stdlib.h>

int main() {
    Instruction p[] = {
        INST_PUSH(3),
        INST_PUSH(3),
        INST_ADD(),

        INST_PRNT(),
    };

    Machine* machine = malloc(sizeof(Machine));
    machine->stackSize = 0;
    machine->program = p;
    machine->programSize = sizeof(p) / sizeof(Instruction);

    RunInstructions(machine);

    free(machine);
    return 0;
}