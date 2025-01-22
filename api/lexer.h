/// Lexer for .pvb files. provrb assembly. might be kinda corny but couldn't
/// think of a name
///
/// Includes token definitions and ways to create
/// Insturction from strings
/// Reads from a .pvb file and executes using the virtual machine

#ifndef LEXER_H
#define LEXER_H

#define USING_ARDUINO // comment this out if not using arduino

/// Create an OpcodeEntry containing an opcode and its string name
/// Easier and more readable than repeating this
#define KW_OP_PAIR(k, o) {.keyword = k, .opcode = o}

#include "inst.h"   // for Instruction and Opcode
#include "macros.h" // for size defintions, i.e MAX_KEYWORD_LEN

typedef enum {
    ERR_INVALID_SYNTAX,
    ERR_RUNTIME_EXCEPTION,
    ERR_TYPE_ERROR,
} Error;

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
    PARSE_LABEL,
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
    unsigned int numTokens;

    Label labels[MAX_LABELS];
    unsigned int numLabels;
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
#ifndef USING_ARDUINO
char* ReadFromFile(char* path, long* stringLength);
#endif

/// Error handling
///
/// .pvb language errors like segmentation faults in c
/// or basic syntax errors

void SyntaxError(Lexer* lexer, char* optMsg);
void TypeError(Lexer* lexer, char* optMsg);

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
void ParseOperands(Lexer* lexer, Opcode opcode, Operand* operands);

/// @brief Get all text on a line number
/// @param lexer - current lexer context
/// @return - all text on the current line number or empty if error
char* GetLine(Lexer* lexer);

/// @brief
/// @param lexer
/// @param opcode
/// @param operand
void CheckOperandSyntax(Lexer* lexer, Opcode opcode, char* operand);

/// @brief Return the number of operands needed for an opcode
/// @param op - opcode to get number of operands for
/// @return - number of operands needed for an opcode to be syntactically
/// correct
int OperandsExpected(Opcode op);

/// @brief Constructor for a Token
/// @param operation - opcode
/// @param keyword - keyword
/// @param operands - list of operands
/// @param lexer - lexer context
/// @return - token
Token NewToken(Opcode operation, char* keyword, Operand* operands, Lexer* lexer);

/// @brief Get an enum Opcode from a string, keyword
/// @param keyword - string to convert into an opcode
/// @return - Opcode representation of keyword or OP_UNKNOWN if failed
Opcode OpcodeFromKeyword(char* keyword);

/// @brief Read a file and try to make tokens out of text
/// @param file - file path to read
#ifndef USING_ARDUINO
Lexer ParseTokens(char* path);
#elif defined(USING_ARDUINO)
Lexer ParseTokens(char* text);
#endif

/// @brief Print all information about a token
/// @param token - token to print information about
void PrintToken(Token* token);

#endif