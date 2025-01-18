#include "api/inst.h"
#include "api/lexer.h"
#include "api/machine.h"
#include "api/macros.h"

#include <stdio.h>
#include <stdlib.h>

int main() {
    ParseTokens("./test.pvb");
    // Machine* machine = malloc(sizeof(Machine));
    // machine->stackSize = 0;
    // machine->ip = 0;

    // Instruction* p = ReadProgramFromFile(machine, "./dump.txt");

    // RunInstructions(machine);

    // free(p);
    // free(machine);
    return 0;
}