/// Lexer for .pvb files. provrb assembly. might be kinda corny but couldn't
/// think of a name
///
/// Includes token definitions and ways to create
/// Insturction from strings
/// Reads from a .pvb file and executes using the virtual machine

#include "inst.h"
#include <stdio.h>

typedef struct {
    Instruction inst;
    char* text;
    char* filepath;
    int line;
} Token;

Token NewToken(Opcode operation, char* keyword, char* operand, char* path,
               long lineNum);

void PrintToken(Token* token);

void SyntaxError(long lineNum, long charNum, char* filePath, char* line);
void ParseTokens(char* file);