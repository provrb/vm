#include "api/lexer.h"
#include <stdio.h>
#include <stdlib.h>

int main() {

    Lexer lexer = ParseTokens("./test.pvb");

    Instruction* insts = malloc(lexer.numTokens * sizeof(Instruction));
    for (unsigned int i = 0; i < lexer.numTokens; i++) {
        insts[i] = lexer.tokens[i].inst;
        PrintToken(&lexer.tokens[i]);
    }

    Machine* machine = malloc(sizeof(Machine));
    machine->stackSize = 0;
    machine->ip = 0;
    machine->program = insts;
    machine->programSize = lexer.numTokens;

    RunInstructions(machine);
    PrintRegisterContents(machine);

    return 0;
}
