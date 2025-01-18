#include "lexer.h"

#include <ctype.h>
#include <libgen.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* ParseKeyword(long* currentCharNum, char* text) {
    char* currentString = malloc(sizeof(char) * 16);
    if (currentString == NULL) {
        fprintf(stderr, "Memory allocaiton failed.\n");
        exit(1);
    }

    char character = text[*currentCharNum];
    long index = 0;
    while (isalpha(character)) {
        // if (character == '\n') {

        //     break;
        // }

        currentString[index] = character;

        index++;
        (*currentCharNum)++;

        character = text[*currentCharNum];
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
    fprintf(stderr, "%s %d:%d: syntax error \n\t(...)%s", filePath, lineNum,
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

// Parse tokens from text
void ParseTokens(char* path) {
    long length = 0;
    long currentCharNum = 0;
    long lineNumber = 1;

    char* text = OpenFile(path, &length);

    while (currentCharNum <= length) {
        char character = text[currentCharNum];

        if (isalpha(character)) {
            // is a token
            char* keyword = ParseKeyword(&currentCharNum, text);
            printf("%d: %s\n", lineNumber, keyword);
            free(keyword);
            continue;
        } else if (isdigit(character)) {
            // is a value
            continue;
        } else {
            char* line = GetLine(currentCharNum, text);
            SyntaxError(lineNumber, currentCharNum, path, line);
        }

        currentCharNum++;

        if (character == '\n') {
            lineNumber++;
            continue;
        }
    }
}