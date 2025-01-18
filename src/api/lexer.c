#include "lexer.h"

#include <ctype.h>
#include <libgen.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* ParseKeyword(long* currentCharNum, char* text) {
    char* currentString = malloc(sizeof(char) * 23);
    if (currentString == NULL) {
        fprintf(stderr, "Memory allocaiton failed.\n");
        exit(1);
    }

    long index = 0;

    while (isalpha(text[*currentCharNum])) {
        char currentChar = text[*currentCharNum];
        currentString[index] = text[*currentCharNum];
        index++;

        (*currentCharNum)++;
    }

    currentString[index] = '\0';
    return currentString;
}

char* GetLine(long currentCharNum, char* text) {
    char character = text[currentCharNum];
    char* line = malloc(sizeof(char) * 40);
    long index = 0;
    while (character != '\n') {
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

void SyntaxError(long lineNum, long charNum, char* filePath, char* line) {
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
    // only one operand operations supported rn
    // get operation from keyword
    int textLen = snprintf(NULL, 0, "%d: %s %s\n", lineNum, keyword, operand);

    Token t = {0};
    t.inst = NewInstruction(IS_PENDING, operation, atoi(operand));
    t.line = lineNum;
    t.filepath = path;
    t.text = malloc(sizeof(char) * textLen);

    snprintf(t.text, textLen, "%d: %s %s\n", lineNum, keyword, operand);

    free(operand);
    free(keyword);
    return t;
}

void PrintToken(Token* token) { printf(""); }

// Parse tokens from text
void ParseTokens(char* path) {
    long length = 0;
    long currentCharNum = 0;
    long lineNumber = 1;

    char* text = OpenFile(path, &length);
    printf("%s\n", text);

    while (currentCharNum < length) {
        if (isalpha(text[currentCharNum])) {
            // is a token
            char* keyword = ParseKeyword(&currentCharNum, text);
            char operand[21] = {0};
            int operandLen = 0;
            if (operand == NULL) {
                fprintf(stderr, "Error allocating memory for operand.\n");
                exit(1);
            }

            // currentCharNum gets set to index after keyword
            // skip spaces, go until not a space
            while (isblank(text[currentCharNum])) {
                // value for token
                // get numeric value. get all digits
                currentCharNum++;
            } 

            while (isdigit(text[currentCharNum])) {
                operand[operandLen] = text[currentCharNum];
                operandLen++;
                currentCharNum++;
            }

            operand[operandLen] = '\0';

            Token token = NewToken(OP_NOP, keyword, operand, path, lineNumber);
            printf("%s\n", token.text);
        } else {
            if (!isspace(text[currentCharNum])) {
                char* line = GetLine(currentCharNum, text);
                SyntaxError(lineNumber, currentCharNum, path, line);
            }
        }

        if (text[currentCharNum] == '\n') {
            lineNumber++;
        }

        currentCharNum++;
    }
}