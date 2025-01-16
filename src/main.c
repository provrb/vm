#include "api/macros.h"
#include "api/machine.h"

int main() {
    Instruction p[] = {
        INST_PUSH(3),
        INST_PUSH(3),
        INST_ADD(),

        INST_PRNT(),
    };

    Machine* machine = NewMachineWithInstructions(p);

    RunInstructions(machine);

    free(machine);
    return 0;
}