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

void SyntaxError(Lexer* lexer) {
    char* line = GetLine(lexer->charIndex, lexer->text);
    fprintf(stderr, "%s %d:%d: syntax error \n\t[...]%s", lexer->filePath,
            lexer->lineNumber, lexer->charIndex, line);

    // print a ^ underneath the bad character
    for (int i = 0; i < 40; i++) {
        if (i == lexer->charIndex)
            printf("\n\t     ^\n");
    }

    free(line);
    free(lexer->filePath);
    exit(-1);
}

// 'keyword' and 'operand' will both be free'd and put into Token::text
Token NewToken(Opcode operation, char* keyword, char* operand, Lexer* lexer) {
    if (operand == NULL) {
        operand = "";
    }

    // only one operand operations supported rn
    // get operation from keyword
    int textLen = snprintf(NULL, 0, "%s %s", keyword, operand);

    Token t = {0};
    t.inst = NewInstruction(IS_PENDING, operation, atoi(operand));
    t.line = lexer->lineNumber;
    t.filepath = lexer->filePath;
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

void SkipLine(Lexer* lexer) {
    while (lexer->text[lexer->charIndex] != '\n' &&
           lexer->charIndex < lexer->textLength) {
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

void ParseTokens(char* path) {

    Lexer lexer = {0};
    char* text = OpenFile(path, &lexer.textLength);

    lexer.lineNumber = 1;
    lexer.filePath = strdup(path);
    lexer.text = strdup(text);

    while (lexer.charIndex < lexer.textLength) {
        // Check for comments first
        if (lexer.text[lexer.charIndex] == LXR_COMMENT) {
            SkipLine(&lexer);
            continue;
        }

        // Check for unknown characters
        if (!isalpha(lexer.text[lexer.charIndex]) &&
            !isspace(lexer.text[lexer.charIndex])) {
            SyntaxError(&lexer);
        }

        // get keyword
        char* keyword = ParseKeyword(&lexer.charIndex, lexer.text);
        if (keyword == NULL) {
            lexer.charIndex++;
            continue;
        }

        Opcode opcode = OpcodeFromKeyword(keyword);
        if (opcode == OP_UNKNOWN)
            SyntaxError(&lexer);

        SkipSpaces(lexer.text,
                   &lexer.charIndex); // skip any spaces between opcode
                                      // keyword and operands

        char* operand = ParseOperand(&lexer.charIndex, lexer.text);

        // check if operand we need and have an operand
        if (OperandExpected(opcode) == TRUE && operand == NULL ||
            OperandExpected(opcode) == FALSE && operand != NULL)
            // no operand!
            SyntaxError(&lexer);

        CheckForComment(&lexer);

        // check if operation requires second operand
        // if so, check for a comma
        // if no comma, syntax error, missing second operand
        // if comma, repeat to parse second operand

        Token token = NewToken(opcode, keyword, operand, &lexer);
        PrintToken(&token);

        if (lexer.text[lexer.charIndex] == '\n')
            lexer.lineNumber++;

        lexer.charIndex++;
    }
}