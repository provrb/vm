#include "api/lexer.h"
#include <stdio.h>
#include <windows.h>

int main() {
    // Start timer
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    LARGE_INTEGER startTime;
    QueryPerformanceCounter(&startTime);

    Lexer lexer = ParseTokens("./test.pvb");

    LARGE_INTEGER endTime;
    QueryPerformanceCounter(&endTime);

    double elapsedTime =
        (double)(endTime.QuadPart - startTime.QuadPart) / frequency.QuadPart * 1000.0;

    Instruction* insts = malloc(lexer.numTokens * sizeof(Instruction));
    for (int i = 0; i < lexer.numTokens; i++) {
        insts[i] = lexer.tokens[i].inst;
        PrintToken(&lexer.tokens[i]);
    }

    Machine* machine = malloc(sizeof(Machine));
    machine->stackSize = 0;
    machine->ip = 0;
    machine->program = insts;
    machine->programSize = lexer.numTokens;

    RunInstructions(machine);

    return 0;
}
