#include "lexer.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/// @brief Array of all the keywords, get the opcode or keyword from eachother
static OpcodeEntry entries[] = {
    KW_OP_PAIR("push", OP_PUSH), KW_OP_PAIR("pop", OP_POP),   KW_OP_PAIR("mov", OP_MOV),
    KW_OP_PAIR("swap", OP_SWAP), KW_OP_PAIR("jmp", OP_JMP),   KW_OP_PAIR("jne", OP_JNE),
    KW_OP_PAIR("je", OP_JE),     KW_OP_PAIR("jg", OP_JG),     KW_OP_PAIR("jge", OP_JGE),
    KW_OP_PAIR("jl", OP_JL),     KW_OP_PAIR("jle", OP_JLE),   KW_OP_PAIR("add", OP_ADD),
    KW_OP_PAIR("sub", OP_SUB),   KW_OP_PAIR("mul", OP_MUL),   KW_OP_PAIR("div", OP_DIV),
    KW_OP_PAIR("mod", OP_MOD),   KW_OP_PAIR("neg", OP_NEG),   KW_OP_PAIR("AND", OP_ANDB),
    KW_OP_PAIR("OR", OP_ORB),    KW_OP_PAIR("NOT", OP_NOTB),  KW_OP_PAIR("XOR", OP_XORB),
    KW_OP_PAIR("shl", OP_SHL),   KW_OP_PAIR("shr", OP_SHR),   KW_OP_PAIR("dup", OP_DUP),
    KW_OP_PAIR("clear", OP_CLR), KW_OP_PAIR("size", OP_SIZE), KW_OP_PAIR("print", OP_PRNT),
};

char* ParseKeyword(Lexer* lexer) {
    char* currentString = malloc(MAX_KEYWORD_LEN * sizeof(char));
    long index = 0;

    while (isalpha(lexer->text[lexer->charIndex])) {
        currentString[index] = lexer->text[lexer->charIndex];
        index++;

        lexer->charIndex++;
    }

    if (index == 0) {
        // no keyword
        free(currentString);
        return NULL;
    }

    currentString[index] = '\0';
    return currentString;
}

char* GetLine(Lexer* lexer) {
    long start = lexer->charIndex;
    long end = lexer->charIndex;

    while (start > 0 && lexer->text[start - 1] != '\n')
        start--;

    while (lexer->text[end] != '\n' && lexer->text[end] != '\0')
        end++;

    long lineLength = end - start;
    char* line = malloc((lineLength + 1) * sizeof(char));
    if (line == NULL)
        return "";

    strncpy(line, &lexer->text[start], lineLength);
    line[lineLength] = '\0';

    return line;
}

char* OpenFile(char* path, long* stringLength) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error opening file. Path: %s\n", path);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = malloc(length + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Buffer allocation error for file contents.\n");
        exit(1);
    }

    fread(buffer, sizeof(char), length / sizeof(char), file);
    fclose(file);

    buffer[length] = '\0';
    *stringLength = length;

    return buffer;
}

int OperandsExpected(Opcode op) {
    switch (op) {
    // operations that require 2 opands
    case OP_MOV:
        return 2;

    // operations that require 1 operand
    case OP_JE:
    case OP_JG:
    case OP_JGE:
    case OP_JL:
    case OP_JLE:
    case OP_JMP:
    case OP_JNE:
    case OP_PUSH:
        return 1;

    // operations that require no operands
    default:
        return 0;
    }

    return -1;
}

void SyntaxError(Lexer* lexer, char* optMsg) {
    char* line = GetLine(lexer);
    fprintf(stderr, "%s:%ld:%ld: syntax error", lexer->filePath, lexer->lineNumber,
            lexer->charIndex);

    if (optMsg) // include optional error message
        fprintf(stderr, " - %s", optMsg);

    fprintf(stderr, "\n\t%s\n\t", line);

    // print a ^ underneath the bad line
    for (int i = 0; i < strlen(line); i++)
        fprintf(stderr, "^");

    fprintf(stderr, "\n");

    free(line);
    free(lexer->filePath);
    exit(-1);
}

// 'keyword' and 'operand' will both be free'd and put into Token::text
Token NewToken(Opcode operation, char* keyword, int* operands, Lexer* lexer) {
    // only one operand operations supported rn
    // get operation from keyword
    int textLen = snprintf(NULL, 0, "%s r%d, r%d", keyword, operands[0], operands[1]) + 1;

    Token t = {0};
    t.line = lexer->lineNumber;
    t.filepath = lexer->filePath;
    t.text = malloc(sizeof(char) * textLen);

    Instruction i = {};
    i.state = IS_PENDING;
    i.operation = operation;

    switch (OperandsExpected(operation)) {
    case 2:
        i.data.registers.src = operands[0];
        i.data.registers.dest = operands[1];
        snprintf(t.text, textLen, "%s r%d, r%d", keyword, i.data.registers.src,
                 i.data.registers.dest);
        break;
    case 1:
        i.data.value = operands[0];
        snprintf(t.text, textLen, "%s %d", keyword, i.data.value);
        break;
    case 0:
        snprintf(t.text, textLen, "%s", keyword);
        break;
    default:
        SyntaxError(lexer, "incorrect number of operands");
    }

    t.inst = i;

    free(keyword);
    return t;
}

void PrintToken(Token* token) { printf("%06d: %s\n", token->line, token->text); }

char* KeywordFromOpcode(Opcode opcode) {
    for (int i = 0; i < sizeof(entries) / sizeof(OpcodeEntry); i++) {
        if (entries[i].opcode == opcode)
            return entries[i].keyword;
    }

    return NULL;
}

Opcode OpcodeFromKeyword(char* keyword) {
    for (int i = 0; i < sizeof(entries) / sizeof(OpcodeEntry); i++) {
        if (strcmp(entries[i].keyword, keyword) == 0)
            return entries[i].opcode;
    }

    return OP_UNKNOWN;
}

void SkipSpaces(Lexer* lexer) {
    // skip spaces, go until not a space
    while (isblank(lexer->text[lexer->charIndex])) {
        // value for token
        // get numeric value. get all digits
        lexer->charIndex++;
    }
}

char* ParseOperand(Lexer* lexer, Opcode opcode) {
    // check if operand
    if (!isdigit(lexer->text[lexer->charIndex])) {
        if (opcode != OP_MOV && lexer->text[lexer->charIndex] != LXR_REG_PREFIX) {
            return NULL; // Return NULL for invalid operand
        }
    }

    int operandLen = 0;
    char* operand = malloc(sizeof(char) * 21);

    // parse operand
    while (isdigit(lexer->text[lexer->charIndex])) {
        operand[operandLen] = lexer->text[lexer->charIndex];
        operandLen++;
        lexer->charIndex++;
    }

    operand[operandLen] = '\0';
    if (operandLen == 0) {
        free(operand);
        return NULL;
    }

    return operand;
}

void SkipLine(Lexer* lexer) {
    while (lexer->text[lexer->charIndex] != '\n' && lexer->charIndex < lexer->textLength) {
        lexer->charIndex++;
    }
    lexer->lineNumber++;
}

void CheckForComment(Lexer* lexer) {
    // check if comment
    if (lexer->text[lexer->charIndex] == LXR_COMMENT) {
        SkipLine(lexer);
    }
}

void CheckOperandSyntax(Lexer* lexer, Opcode opcode, char* operand) {
    // check if operand we need and have an operand
    if ((OperandsExpected(opcode) != 0) != (operand != NULL)) {
        if (OperandsExpected(opcode) > 0 && !operand)
            SyntaxError(lexer, "missing operand");
        else if (OperandsExpected(opcode) == 0 && operand)
            SyntaxError(lexer, "passing unneccessary operand");
        else
            SyntaxError(lexer, NULL);
    }
}

void ParseOperands(Lexer* lexer, Opcode opcode, int* operands) {
    // parse 2 operands maximum
    for (int opIndex = 0; opIndex < OperandsExpected(opcode); opIndex++) {
        if (lexer->text[lexer->charIndex] == LXR_OPRND_BRK) { // ","
            lexer->charIndex++;
            SkipSpaces(lexer);
        } else if (opIndex > 0 && opcode == OP_MOV &&
                   lexer->text[lexer->charIndex] != LXR_OPRND_BRK) {
            SyntaxError(lexer, "missing seperator between operands");
        }

        if (opcode == OP_MOV && lexer->text[lexer->charIndex] == LXR_REG_PREFIX) {
            lexer->charIndex++;
        } else if (opcode == OP_MOV && lexer->text[lexer->charIndex] != LXR_REG_PREFIX)
            SyntaxError(lexer, "no register prefix");

        // Parse the operand associated with the opcode
        char* operand = ParseOperand(lexer, opcode);

        // Check the syntax of the operand
        CheckOperandSyntax(lexer, opcode, operand);

        CheckForComment(lexer); // i dont't know why but we need to check
                                // for comments twice. it works so.

        operands[opIndex] = atoi(operand);
    }
}

void SkipWhitespace(Lexer* lexer) {
    while (isspace(lexer->text[lexer->charIndex])) {
        if (lexer->text[lexer->charIndex] == '\n')
            lexer->lineNumber++;
        lexer->charIndex++;
    }
}

void ParseTokens(char* path) {
    // Open file and load its contents
    long tl = 0;
    char* text = OpenFile(path, &tl);

    // Create lexxer struct from known variables
    Lexer lexer = {
        .lineNumber = 1, .filePath = strdup(path), .text = strdup(text), .textLength = tl};

    free(text);

    while (lexer.charIndex < lexer.textLength) {
        // skip any whitespace and newlines
        SkipWhitespace(&lexer);

        // Check for comments
        CheckForComment(&lexer);

        // Check for unknown characters
        if (!isalpha(lexer.text[lexer.charIndex]) && !isspace(lexer.text[lexer.charIndex])) {
            if (lexer.text[lexer.charIndex] == '\0') // last character in file
                break;

            SyntaxError(&lexer, "unknown character");
        }

        // get keyword
        char* keyword = ParseKeyword(&lexer);
        if (keyword == NULL) {
            lexer.charIndex++;
            continue;
        }

        // get opcode from keyword as an Opcode enum
        Opcode opcode = OpcodeFromKeyword(keyword);
        if (opcode == OP_UNKNOWN)
            SyntaxError(&lexer, "unknown opcode");

        // Skip whitespace between opcode and operand
        SkipSpaces(&lexer); // skip any spaces between opcode
                            // keyword and operands

        int operands[2] = {0};
        ParseOperands(&lexer, opcode, operands);

        // Create token from keyword, opcode, and operands
        Token token = NewToken(opcode, keyword, operands, &lexer);
        lexer.tokens[lexer.numTokens++] = token; // append token to array of tokens

        // move to the next character
        lexer.charIndex++;
    }

    for (int ti = 0; ti < lexer.numTokens; ti++) {
        PrintToken(&lexer.tokens[ti]);
    }
}