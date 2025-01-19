#include "lexer.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Opcode OpcodeFromKeyword(char* keyword) {
    if (strcmp(keyword, "nop") == 0)
        return OP_NOP;
    else if (strcmp(keyword, "push") == 0)
        return OP_PUSH;
    else if (strcmp(keyword, "pop") == 0)
        return OP_POP;
    else if (strcmp(keyword, "mov") == 0)
        return OP_MOV;
    else if (strcmp(keyword, "swap") == 0)
        return OP_SWAP;
    else if (strcmp(keyword, "jmp") == 0)
        return OP_JMP;
    else if (strcmp(keyword, "jne") == 0)
        return OP_JNE;
    else if (strcmp(keyword, "je") == 0)
        return OP_JE;
    else if (strcmp(keyword, "jg") == 0)
        return OP_JG;
    else if (strcmp(keyword, "jge") == 0)
        return OP_JGE;
    else if (strcmp(keyword, "jl") == 0)
        return OP_JL;
    else if (strcmp(keyword, "jle") == 0)
        return OP_JLE;
    else if (strcmp(keyword, "add") == 0)
        return OP_ADD;
    else if (strcmp(keyword, "sub") == 0)
        return OP_SUB;
    else if (strcmp(keyword, "mul") == 0)
        return OP_MUL;
    else if (strcmp(keyword, "div") == 0)
        return OP_DIV;
    else if (strcmp(keyword, "mod") == 0)
        return OP_MOD;
    else if (strcmp(keyword, "neg") == 0)
        return OP_NEG;
    else if (strcmp(keyword, "AND") == 0)
        return OP_ANDB;
    else if (strcmp(keyword, "OR") == 0)
        return OP_ORB;
    else if (strcmp(keyword, "NOT") == 0)
        return OP_NOTB;
    else if (strcmp(keyword, "XOR") == 0)
        return OP_XORB;
    else if (strcmp(keyword, "shl") == 0)
        return OP_SHL;
    else if (strcmp(keyword, "shr") == 0)
        return OP_SHR;
    else if (strcmp(keyword, "dup") == 0)
        return OP_DUP;
    else if (strcmp(keyword, "clear") == 0)
        return OP_CLR;
    else if (strcmp(keyword, "size") == 0)
        return OP_SIZE;
    else if (strcmp(keyword, "print") == 0)
        return OP_PRNT;

    return OP_UNKNOWN; // if the keyword doesn't match any known opcode
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

char* ReadFromFile(char* path, long* stringLength) {
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
    for (size_t i = 0; i < strlen(line); i++)
        fprintf(stderr, "^");

    fprintf(stderr, "\n");

    free(line);
    free(lexer->filePath);
    exit(-1);
}

void TypeError(Lexer* lexer, char* optMsg) {
    char* line = GetLine(lexer);
    fprintf(stderr, "%s:%ld:%ld: type error", lexer->filePath, lexer->lineNumber, lexer->charIndex);

    if (optMsg) // include optional error message
        fprintf(stderr, " - %s", optMsg);

    fprintf(stderr, "\n\t%s\n\t", line);

    // print a ^ underneath the bad line
    for (size_t i = 0; i < strlen(line); i++)
        fprintf(stderr, "^");

    fprintf(stderr, "\n");

    free(line);
    free(lexer->filePath);
    exit(-1);
}

// 'keyword' and 'operand' will both be free'd and put into Token::text
Token NewToken(Opcode operation, char* keyword, Operand* operands, Lexer* lexer) {
    // only one operand operations supported rn
    // get operation from keyword
    int textLen = 1024;

    Token t = {0};
    t.line = lexer->lineNumber;
    t.filepath = lexer->filePath;
    t.text = malloc(sizeof(char) * textLen);

    Instruction i = {};
    i.state = IS_PENDING;
    i.operation = operation;

    switch (OperandsExpected(operation)) {
    case 2:
        if (operands[0].type != TY_I64 || operands[1].type != TY_I64)
            TypeError(lexer, "expected I64 operand");

        i.data.registers.src = operands[0].data.i64;
        i.data.registers.dest = operands[1].data.i64;
        snprintf(t.text, textLen, "%s r%d, r%d", keyword, i.data.registers.src,
                 i.data.registers.dest);
        break;
    case 1:
        i.data.value = operands[0];
        if (operands[0].type == TY_STR)
            snprintf(t.text, textLen, "%s \"%s\"", keyword, (char*)i.data.value.data.ptr);
        else if (operands[0].type == TY_I64 || operands[0].type == TY_U64)
            snprintf(t.text, textLen, "%s %ld", keyword, i.data.value.data.i64);

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

char* ParseOperand(Lexer* lexer, Opcode opcode) {

    // check if operand
    if (!isdigit(lexer->text[lexer->charIndex])) {
        if (opcode != OP_MOV && lexer->text[lexer->charIndex] != LXR_REG_PREFIX &&
            opcode != OP_PUSH && lexer->text[lexer->charIndex] != LXR_STR_CHAR) {
            return NULL; // Return NULL for invalid operand
        }
    }

    if (opcode == OP_PUSH && lexer->text[lexer->charIndex] == LXR_STR_CHAR) {
        int stringChars = 1;
        char* string = malloc((STACK_CAPACITY / 4) * sizeof(char));
        int stringIndex = 0;

        lexer->charIndex++;
        // iterate until next string character
        while (lexer->text[lexer->charIndex] != '\0') {
            if (lexer->text[lexer->charIndex] == LXR_STR_CHAR) {
                stringChars++;
                if (stringChars % 2 == 0) {
                    lexer->charIndex++; // Closing quotation mark
                    break;
                }
            } else {
                string[stringIndex++] = lexer->text[lexer->charIndex];
            }
            lexer->charIndex++;
        }
        string[stringIndex] = '\0';

        // strings in pairs
        if (stringChars % 2 != 0) {
            free(string);
            SyntaxError(lexer, "missing quotation mark");
        }
        return string;
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

void ParseOperands(Lexer* lexer, Opcode opcode, Operand* operands) {
    // parse 2 operands maximum
    for (int opIndex = 0; opIndex < OperandsExpected(opcode); opIndex++) {
        if (lexer->text[lexer->charIndex] == LXR_OPRND_BRK) { // ","
            lexer->charIndex++;
            SkipSpaces(lexer);
        } else if (opIndex > 0 && opcode == OP_MOV &&
                   lexer->text[lexer->charIndex] != LXR_OPRND_BRK)
            SyntaxError(lexer, "missing seperator between operands");

        if (opcode == OP_MOV && lexer->text[lexer->charIndex] == LXR_REG_PREFIX)
            lexer->charIndex++;
        else if (opcode == OP_MOV && lexer->text[lexer->charIndex] != LXR_REG_PREFIX)
            SyntaxError(lexer, "no register prefix");

        // Parse the operand associated with the opcode
        char* operand = ParseOperand(lexer, opcode);
        if (atoi(operand) == 0 && operand == "0") {
            operands[opIndex].data.ptr = operand;
            operands[opIndex].type = TY_STR;
        } else {
            operands[opIndex].data.i64 = atoi(operand);
            operands[opIndex].type = TY_I64;
        }

        // Check the syntax of the operand
        CheckOperandSyntax(lexer, opcode, operand);
    }
}

void SkipSpaces(Lexer* lexer) {
    // skip spaces, go until not a space
    while (isblank(lexer->text[lexer->charIndex])) {
        // value for token
        // get numeric value. get all digits
        lexer->charIndex++;
    }
}

Lexer ParseTokens(char* path) {
    // Open file and load its contents
    long tl = 0;
    char* text = ReadFromFile(path, &tl);

    // Create lexxer struct from known variables
    Lexer lexer = {
        .state = PARSE, .lineNumber = 1, .filePath = path, .text = text, .textLength = tl};

    char keyword[MAX_KEYWORD_LEN] = {0};
    long index = 0;

    while (lexer.charIndex <= lexer.textLength && lexer.numTokens <= MAX_PROGRAM_SIZE) {
        if (lexer.state == SKIP_SPACES) {
            if (isblank(lexer.text[lexer.charIndex])) {
                lexer.charIndex++;
                continue;
            }
            lexer.state = PARSE;
        }

        // is comment
        if (lexer.text[lexer.charIndex] == LXR_COMMENT && lexer.state != SKIP_LINE) {
            lexer.state = SKIP_LINE;
            lexer.charIndex++;
        }

        // skipping line
        if (lexer.state == SKIP_LINE && lexer.text[lexer.charIndex] != '\n') {
            lexer.charIndex++;
            continue;
        } else if (lexer.state == SKIP_LINE && lexer.text[lexer.charIndex] == '\n') {
            lexer.charIndex++;
            lexer.lineNumber++;
            lexer.state = PARSE;
            continue;
        }

        // detect unknonwn characters
        if (lexer.charIndex < lexer.textLength && !isalpha(lexer.text[lexer.charIndex]) &&
            !isspace(lexer.text[lexer.charIndex])) {
            if (lexer.text[lexer.charIndex] != LXR_STR_CHAR)
                SyntaxError(&lexer, "unknown character");
        }

        // We are in general parsing state
        if (lexer.state == PARSE) {
            lexer.state = PARSE_KWD;
            index = 0;
            memset(keyword, 0, MAX_KEYWORD_LEN);
        }

        // Parse a keyword
        if (lexer.state == PARSE_KWD) {
            if (lexer.charIndex < lexer.textLength && isalpha(lexer.text[lexer.charIndex])) {
                keyword[index++] = lexer.text[lexer.charIndex];
                lexer.charIndex++;
                continue;
            }

            if (index == 0) {
                lexer.charIndex++;
                continue;
            }

            keyword[index] = '\0';
            lexer.state = PARSE;
        }

        // get opcode from keyword as an Opcode enum
        Opcode opcode = OpcodeFromKeyword(keyword);
        if (opcode == OP_UNKNOWN)
            SyntaxError(&lexer, "unknown opcode");

        SkipSpaces(&lexer); // skip any spaces between opcode keyword and operands

        Operand operands[2] = {0};
        ParseOperands(&lexer, opcode, operands);

        // Create token from keyword, opcode, and operands
        Token token = NewToken(opcode, strdup(keyword), operands, &lexer);
        lexer.tokens[lexer.numTokens++] = token; // append token to array of tokens
    }

    printf("Parsed %d instructions.\n", lexer.numTokens);

    free(text);
    free(path);
    return lexer;
}