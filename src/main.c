#include "lexer.h"

#ifndef USING_ARDUINO
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    if (argc == 1) {
        fprintf(stderr,
                "Insufficient amount of arguments passed. No file path specified. Aborted.\n");
        exit(1);
    }

    Lexer lexer = ParseTokens(argv[1]);

    Instruction* insts = malloc(lexer.numTokens * sizeof(Instruction));
    for (unsigned int i = 0; i < lexer.numTokens; i++) {
        insts[i] = lexer.tokens[i].inst;
        PrintToken(&lexer.tokens[i]);
    }

    Machine* machine = malloc(sizeof(Machine));
    machine->stackSize = 0;
    machine->ip = 0;
    machine->program = insts;
    machine->rp = -1;
    machine->programSize = lexer.numTokens;
    for (int i = 0; i < lexer.numLabels; i++) {
        machine->labels[i] = lexer.labels[i];
    }

    machine->numLabels = lexer.numLabels;

    RunInstructions(machine);
    PrintRegisterContents(machine);

    return 0;
}

#endif