/// Lexer for .pvb files. provrb assembly. might be kinda corny but couldn't
/// think of a name
///
/// Includes token definitions and ways to create
/// Insturction from strings
/// Reads from a .pvb file and executes using the virtual machine

#ifndef LEXER_H
#define LEXER_H

/// Create an OpcodeEntry containing an opcode and its string name
/// Easier and more readable than repeating this
#define KW_OP_PAIR(k, o) {.keyword = k, .opcode = o}

#include "inst.h"   // for Instruction and Opcode
#include "macros.h" // for size defintions, i.e MAX_KEYWORD_LEN

/// @brief More information about an instruction
///
/// Includes the parsed instruction information
/// file that the instruction was read from
/// line number the instruction is on
/// and the actual raw line contents, e.g 'push 8'
typedef struct {
    Instruction inst;
    char* text;
    char* filepath;
    int line;
} Token;

typedef struct {
    long charIndex;
    char* text;
    long textLength;
    long lineNumber;
    char* filePath;
    Token tokens[MAX_PROGRAM_SIZE];
    int numTokens;
} Lexer;

/// @brief An entry to represent relationship between string and enum
///
/// Map opcode to string representation
/// Map string keyword to opcode
/// Necessary when parsing language
typedef struct {
    char keyword[MAX_KEYWORD_LEN];
    Opcode opcode;
} OpcodeEntry;

Token NewToken(Opcode operation, char* keyword, int* operands, Lexer* lexer);
char* KeywordFromOpcode(Opcode opcode);
Opcode OpcodeFromKeyword(char* keyword);
void PrintToken(Token* token);
void SyntaxError(Lexer* lexer, char* optMsg);
void ParseTokens(char* file);

#endif