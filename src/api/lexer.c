#include "lexer.h"

#include <stdio.h>
#include <stdlib.h>

char* ParseKeyword(long* currentCharNum, char* text) {
    char* currentString = malloc(MAX_KEYWORD_LEN * sizeof(char));
    long index = 0;

    while (isalpha(text[*currentCharNum])) {
        currentString[index] = text[*currentCharNum];
        index++;

        (*currentCharNum)++;
    }

    if (index == 0) {
        // no keyword
        free(currentString);
        return NULL;
    }

    currentString[index] = '\0';
    return currentString;
}

char* GetLine(long currentCharNum, char* text) {
    long index = 0;
    char* line = malloc(LXR_MAX_LINE_LEN * sizeof(char));
    if (line == NULL) {
        fprintf(stderr, "Memory allocation failed for line buffer.\n");
        exit(1);
    }

    char character = text[currentCharNum];

    while (character != '\n' && character != '\0') {
        if (index >= LXR_MAX_LINE_LEN - 1) {
            line = realloc(line, (LXR_MAX_LINE_LEN * 2) *
                                     sizeof(char)); // Double the space
            if (line == NULL) {
                fprintf(stderr,
                        "Memory reallocation failed for line buffer.\n");
                exit(1);
            }
        }

        line[index] = character;
        index++;
        currentCharNum++;
        character = text[currentCharNum];
    }

    line[index] = '\0';

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

int OperandExpected(Opcode op) {
    switch (op) {
    case OP_UNKNOWN:
    case OP_NOP:
    case OP_NEG:
    case OP_DUP:
    case OP_PRNT:
    case OP_POP:
    case OP_CLR:
    case OP_SIZE:
        return FALSE;
    }

    return TRUE;
}

void SyntaxError(char* text, long lineNum, long charNum, char* filePath) {
    char* line = GetLine(charNum, text);
    fprintf(stderr, "%s %d:%d: syntax error \n\t[...]%s", filePath, lineNum,
            charNum, line);

    // print a ^ underneath the bad character
    for (int i = 0; i < 40; i++) {
        if (i == charNum)
            printf("\n\t     ^\n");
    }

    free(line);
    free(filePath);
    exit(-1);
}

// 'keyword' and 'operand' will both be free'd and put into Token::text
Token NewToken(Opcode operation, char* keyword, char* operand, char* path,
               long lineNum) {
    if (operand == NULL) {
        operand = "";
    }

    // only one operand operations supported rn
    // get operation from keyword
    int textLen = snprintf(NULL, 0, "%s %s", keyword, operand);

    Token t = {0};
    t.inst = NewInstruction(IS_PENDING, operation, atoi(operand));
    t.line = lineNum;
    t.filepath = path;
    t.text = malloc(sizeof(char) * textLen);

    snprintf(t.text, textLen, "%s %s", keyword, operand);

    free(operand);
    free(keyword);
    return t;
}

void PrintToken(Token* token) {
    printf("%06d: %s (%d %d)\n", token->line, token->text,
           token->inst.operation, token->inst.data.value);
}

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

void SkipSpaces(char* text, long* charNum) {
    // skip spaces, go until not a space
    if (isblank(text[*charNum])) {
        while (isblank(text[*charNum])) {
            // value for token
            // get numeric value. get all digits
            (*charNum)++;
        }
    }
}

char* ParseOperand(long* charNum, char* text) {
    // check if operand
    if (!isdigit(text[*charNum])) {
        return NULL;
    }

    int operandLen = 0;
    char* operand = malloc(sizeof(char) * 21);

    // parse operand
    while (isdigit(text[*charNum])) {
        operand[operandLen] = text[*charNum];
        operandLen++;
        (*charNum)++;
    }

    operand[operandLen] = '\0';
    if (operandLen == 0) {
        free(operand);
        return NULL;
    }

    return operand;
}

// Parse tokens from text
void ParseTokens(char* path) {
    long length = 0;
    long currentCharNum = 0;
    long lineNumber = 1;

    char* text = OpenFile(path, &length);

    while (currentCharNum < length) {
        // check if comment
        if (text[currentCharNum] == LXR_COMMENT) {
            // skip line
            while (text[currentCharNum] != '\n') {
                currentCharNum++;
            }
            continue;
        }

        if (isalpha(text[currentCharNum])) {
            // get keyword
            char* keyword = ParseKeyword(&currentCharNum, text);
            if (keyword == NULL) {
                currentCharNum++;
                continue;
            }

            Opcode opcode = OpcodeFromKeyword(keyword);
            if (opcode == OP_UNKNOWN)
                SyntaxError(text, lineNumber, currentCharNum, path);

            SkipSpaces(text, &currentCharNum); // skip any spaces between opcode
                                               // keyword and operands

            char* operand = ParseOperand(&currentCharNum, text);

            // check if operand we need and have an operand
            if (OperandExpected(opcode) == TRUE && operand == NULL ||
                OperandExpected(opcode) == FALSE && operand != NULL)
                // no operand!
                SyntaxError(text, lineNumber, currentCharNum, path);

            // check if operation requires second operand
            // if so, check for a comma
            // if no comma, syntax error, missing second operand
            // if comma, repeat to parse second operand

            Token token = NewToken(opcode, keyword, operand, path, lineNumber);
            PrintToken(&token);
        } else {
            if (!isspace(text[currentCharNum]) &&
                text[currentCharNum] != LXR_COMMENT) {
                SyntaxError(text, lineNumber, currentCharNum, path);
            }
        }

        if (text[currentCharNum] == '\n') {
            lineNumber++;
        }

        currentCharNum++;
    }
}