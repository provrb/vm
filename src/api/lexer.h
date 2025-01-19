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

typedef enum {
    NONE = 0,
    PARSE,
    PARSE_KWD,
    PARSE_OPND,
    SKIP_LINE,
    SKIP_SPACES,
} LexerState;

typedef struct {
    long charIndex;
    char* text;
    long textLength;
    long lineNumber;
    char* filePath;
    LexerState state;
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

/// File I/O operations
char* OpenFile(char* path, long* stringLength);

/// Error handling
///
/// .pvb language errors like segmentation faults in c
/// or basic syntax errors

void SyntaxError(Lexer* lexer, char* optMsg);

/// Lexical analysis
///
/// Functions related to the lexer/parsing stage
/// These include: ParseKeyword, ParseOperands, etc

/// @brief Skip all blank characters until a non-blank character is reached
/// @param lexer - current lexer context
void SkipSpaces(Lexer* lexer);

/// @brief Parse one operand for an opcode instruction
/// @param lexer - current lexer context
/// @param opcode - opcode to search operands for
/// @return - string version of one operand
char* ParseOperand(Lexer* lexer, Opcode opcode);

/// @brief Parse operands required for an operation
/// @param lexer - current lexer context
/// @param opcode - opcode
/// @param operands - out array that stores the number operands
void ParseOperands(Lexer* lexer, Opcode opcode, int* operands);

/// @brief Parse the current word as a language keyword
/// @param lexer - current lexer context
/// @return - string keyword
char* ParseKeyword(Lexer* lexer);

/// @brief Get all text on a line number
/// @param lexer - current lexer context
/// @return - all text on the current line number or empty if error
char* GetLine(Lexer* lexer);

/// @brief Skip over any characters until the \n character is reached
/// @param lexer - current lexer context
void SkipLine(Lexer* lexer);

/// Check if the current character is a comment escape character
/// if so, disregard everything after the comment character on the line
/// @param lexer - current lexer context
void CheckForComment(Lexer* lexer);

/// @brief
/// @param lexer
/// @param opcode
/// @param operand
void CheckOperandSyntax(Lexer* lexer, Opcode opcode, char* operand);

/// @brief Return the number of operands needed for an opcode
/// @param op - opcode to get number of operands for
/// @return - number of operands needed for an opcode to be syntactically correct
int OperandsExpected(Opcode op);

/// @brief Constructor for a Token
/// @param operation - opcode
/// @param keyword - keyword
/// @param operands - list of operands
/// @param lexer - lexer context
/// @return - token
Token NewToken(Opcode operation, char* keyword, int* operands, Lexer* lexer);

/// @brief Get an enum Opcode from a string, keyword
/// @param keyword - string to convert into an opcode
/// @return - Opcode representation of keyword or OP_UNKNOWN if failed
Opcode OpcodeFromKeyword(char* keyword);

/// @brief Get the string keyword from an enum Opcode
/// @param opcode - opcode to get the keyword of
/// @return - keyword (i.e "push")
char* KeywordFromOpcode(Opcode opcode);

/// @brief Read a file and try to make tokens out of text
/// @param file - file path to read
void ParseTokens(char* file);

/// @brief Print all information about a token
/// @param token - token to print information about
void PrintToken(Token* token);

#endif