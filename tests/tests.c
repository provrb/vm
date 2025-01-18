#include <assert.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#include "../src/api/inst.h"
#include "../src/api/machine.h"
#include "../src/api/macros.h"

int main() {

    Machine* machine = malloc(sizeof(Machine));
    machine->stackSize = 0;
    machine->ip = 0;

    {
        // Push test
        Instruction a[] = {
            INST_PUSH(3),
            INST_PUSH(2),
            INST_PUSH(7),
        };

        machine->program = a;
        machine->programSize = sizeof(a) / sizeof(Instruction);

        RunInstructions(machine);
        assert((machine->stack[0] == 3 && machine->stack[1] == 2 && machine->stack[2] == 7) &&
               "Test failed for PUSH instruction.");
    }

    printf("PUSH test passed.\n");
    Reset(machine);

    {
        // Pop test
        Instruction b[] = {
            INST_PUSH(3),
            INST_POP(),
            INST_PUSH(9),
        };

        machine->program = b;
        machine->programSize = sizeof(b) / sizeof(Instruction);

        RunInstructions(machine);
        assert(machine->stack[0] == 9 && "Test failed for POP instruction.");
    }

    printf("POP test passed.\n");
    Reset(machine);

    {
        // Move test
        Instruction p[] = {
            INST_PUSH(1), INST_PUSH(2), INST_PUSH(3), INST_PUSH(4), INST_MOV(3, 2),
        };

        machine->program = p;
        machine->programSize = sizeof(p) / sizeof(Instruction);

        RunInstructions(machine);
        assert(machine->stack[2] == 4 && "Test failed for MOV instruction.");
    }

    printf("MOV test passed.\n");
    Reset(machine);

    {
        // Clear test
        Instruction p[] = {
            INST_PUSH(1), INST_PUSH(2), INST_PUSH(3), INST_PUSH(4), INST_CLR(),
        };

        machine->program = p;
        machine->programSize = sizeof(p) / sizeof(Instruction);

        RunInstructions(machine);
        assert(machine->stackSize == 0 && "Test failed for CLR instruction.");
    }

    printf("CLR test passed.\n");
    Reset(machine);

    free(machine);

    printf("All tests passed.\n");
    return 0;
}