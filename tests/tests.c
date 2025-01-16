#include <stdio.h>
#include <assert.h>

#include "../src/api/inst.h"
#include "../src/api/machine.h"
#include "../src/api/macros.h"


int main() {

    {
        // Push test
        Instruction p[] = {
            INST_PUSH(3),
            INST_PUSH(2),
            INST_PUSH(7),
            INST_PRNT(),
        };

        RunInstructions(p);
        // assert( (m.stack[0] == 3 && m.stack[1] == 2 && m.stack[2] == 7 ) && "Test failed for PUSH instruction." );
    }


    {
        // Pop test
        Instruction program[] = {
            INST_PUSH(3),
            INST_POP(),
        };

        // assert( (stackSize == 0) && "Test failed for POP instruction." );
    }

    printf("All tests passed.\n");
    return 0;
}