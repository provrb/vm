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

/// @brief Array of all the keywords, get the opcode or keyword from eachother
static OpcodeEntry entries[] = {
    KW_OP_PAIR("push", OP_PUSH),  KW_OP_PAIR("pop", OP_POP),
    KW_OP_PAIR("mov", OP_MOV),    KW_OP_PAIR("swap", OP_SWAP),
    KW_OP_PAIR("jmp", OP_JMP),    KW_OP_PAIR("jne", OP_JNE),
    KW_OP_PAIR("je", OP_JE),      KW_OP_PAIR("jg", OP_JG),
    KW_OP_PAIR("jge", OP_JGE),    KW_OP_PAIR("jl", OP_JL),
    KW_OP_PAIR("jle", OP_JLE),    KW_OP_PAIR("add", OP_ADD),
    KW_OP_PAIR("sub", OP_SUB),    KW_OP_PAIR("mul", OP_MUL),
    KW_OP_PAIR("div", OP_DIV),    KW_OP_PAIR("mod", OP_MOD),
    KW_OP_PAIR("neg", OP_NEG),    KW_OP_PAIR("AND", OP_ANDB),
    KW_OP_PAIR("OR", OP_ORB),     KW_OP_PAIR("NOT", OP_NOTB),
    KW_OP_PAIR("XOR", OP_XORB),   KW_OP_PAIR("shl", OP_SHL),
    KW_OP_PAIR("shr", OP_SHR),    KW_OP_PAIR("dup", OP_DUP),
    KW_OP_PAIR("clear", OP_CLR),  KW_OP_PAIR("size", OP_SIZE),
    KW_OP_PAIR("print", OP_PRNT),
};

Token NewToken(Opcode operation, char* keyword, char* operand, Lexer* lexer);
char* KeywordFromOpcode(Opcode opcode);
Opcode OpcodeFromKeyword(char* keyword);
void PrintToken(Token* token);
void SyntaxError(Lexer* lexer);
void ParseTokens(char* file);

#endif