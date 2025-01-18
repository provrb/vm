#include "inst.h"

#include <stdio.h>

typedef struct {
    Instruction inst;
    char* text;
    char* filepath;
    int line;
    int character;
} Token;

void SyntaxError(long lineNum, long charNum, char* filePath, char* line);
void ParseTokens(char* file);