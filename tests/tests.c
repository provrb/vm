#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <memory.h>

#include "../src/api/inst.h"
#include "../src/api/machine.h"
#include "../src/api/macros.h"

void Reset(Machine* m) {
    int empty[1024];
    m->stackSize = 0;
    memcpy(m->stack, empty, sizeof(empty));
}

int main() {

    Machine* machine = malloc(sizeof(Machine));
    {
        // Push test
        Instruction p[] = {
            INST_PUSH(3),
            INST_PUSH(2),
            INST_PUSH(7),
        };

        machine->program = p;

        RunInstructions(machine);
        assert( (machine->stack[0] == 3 && machine->stack[1] == 2 && machine->stack[2] == 7 ) && "Test failed for PUSH instruction." );
    }

    printf("PUSH test passed.\n");
    Reset(machine);

    {
        // Pop test
        Instruction p[] = {
            INST_PUSH(3),
            INST_POP(),
            INST_PUSH(9),
        };
        machine->program = p;
        RunInstructions(machine);
        assert(machine->stack[0] == 9 && "Test failed for POP instruction.");
    }

    printf("POP test passed.\n");
    Reset(machine);
    {
        
    }

    printf("All tests passed.\n");
    return 0;
}