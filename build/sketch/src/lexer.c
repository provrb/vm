#line 1 "/home/ethan/Documents/provrb/vm/src/lexer.c"
#include "../api/lexer.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const OpcodeEntry opcodeTable[] = {
    {"nop", OP_NOP},   {"push", OP_PUSH},    {"pop", OP_POP},   {"mov", OP_MOV},
    {"swap", OP_SWAP}, {"jmp", OP_JMP},      {"jne", OP_JNE},   {"je", OP_JE},
    {"jg", OP_JG},     {"jge", OP_JGE},      {"jl", OP_JL},     {"jle", OP_JLE},
    {"add", OP_ADD},   {"sub", OP_SUB},      {"mul", OP_MUL},   {"div", OP_DIV},
    {"mod", OP_MOD},   {"neg", OP_NEG},      {"AND", OP_ANDB},  {"OR", OP_ORB},
    {"NOT", OP_NOTB},  {"XOR", OP_XORB},     {"shl", OP_SHL},   {"shr", OP_SHR},
    {"dup", OP_DUP},   {"clear", OP_CLR},    {"size", OP_SIZE}, {"print", OP_PRNT},
    {"exit", OP_EXIT}, {"null", OP_UNKNOWN},
};

Opcode OpcodeFromKeyword(char* keyword) {
    for (int i = 0; opcodeTable[i].opcode != OP_UNKNOWN; i++)
        if (strcmp(opcodeTable[i].keyword, keyword) == 0)
            return opcodeTable[i].opcode;

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

#ifndef USING_ARDUINO
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
#endif

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

    fprintf(stderr, " here\n");

    free(line);
    exit(ERR_INVALID_SYNTAX);
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
    exit(ERR_TYPE_ERROR);
}

// 'keyword' and 'operand' will both be free'd and put into Token::text
Token NewToken(Opcode operation, char* keyword, Operand* operands, Lexer* lexer) {
    int textLen = 1024;

    Token t = {0};
    t.line = lexer->lineNumber;
    t.filepath = lexer->filePath;
    t.text = malloc(sizeof(char) * textLen);

    Instruction i = {};
    i.operation = operation;

    switch (OperandsExpected(operation)) {
    case 2:
        if (operation == OP_MOV) {
            if (operands[0].type == TY_STR &&
                ((char*)operands[0].data.ptr)[0] == LXR_CONSTANT_PREFIX) {
                printf("Is a constant\n");
                i.data.value.data.ptr = operands[0].data.ptr;
                i.data.registers.dest = operands[1].data.i64;
                printf("dest is %ld\n", i.data.registers.dest);
                printf("constant is %s\n", (char*)i.data.value.data.ptr);

                snprintf(t.text, textLen, "%s $%d, %s", keyword, operands[0].data.i64,
                         GetRegisterName(i.data.registers.dest));

                break;
            }
        }

        if (operation != OP_MOV && (operands[0].type != TY_I64 || operands[1].type != TY_I64))
            TypeError(lexer, "expected I64 operand");

        i.data.value.data.i64 = operands[0].data.i64;
        i.data.value.type = operands[0].type;

        i.data.registers.src = operands[0].data.i64;
        i.data.registers.dest = operands[1].data.i64;
        snprintf(t.text, textLen, "%s %s, %s", keyword, GetRegisterName(i.data.registers.src),
                 GetRegisterName(i.data.registers.dest));
        break;
    case 1:
        if (i.operation == OP_PUSH && operands[0].type == TY_STR &&
            GetRegisterFromName((char*)operands[0].data.ptr) != REG_UNKNOWN) {
            // operand is a register
            printf("On a register\n");
            snprintf(t.text, textLen, "%s %s", keyword, (char*)operands[0].data.ptr);
            i.data.value.data.i64 = operands[0].data.i64;
            i.data.value.type = TY_STR;
            break;
        }

        i.data.value = operands[0];

        if (operands[0].type == TY_STR)
            snprintf(t.text, textLen, "%s \"%s\"", keyword, (char*)i.data.value.data.ptr);
        else if (operands[0].type == TY_I64 || operands[0].type == TY_U64)
            snprintf(t.text, textLen, "%s %ld", keyword, i.data.value.data.i64);

        break;
    case 0:
        // pop can have an optional operand
        if (i.operation == OP_POP && operands[0].type == TY_STR) {
            snprintf(t.text, textLen, "%s %s", keyword, (char*)operands[0].data.ptr);
            i.data.registers.dest = GetRegisterFromName((char*)operands[0].data.ptr);
            break;
        }

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
    if (lexer->text[lexer->charIndex] == LXR_CONSTANT_PREFIX && opcode == OP_MOV) {
        printf("Parsing constant\n");
        char* operand = malloc(MAX_OPERAND_LEN * sizeof(char));
        int operandIndex = 0;
        operand[operandIndex++] = LXR_CONSTANT_PREFIX;

        lexer->charIndex++;
        while (isdigit(lexer->text[lexer->charIndex])) {
            operand[operandIndex] = lexer->text[lexer->charIndex];
            operandIndex++;
            lexer->charIndex++;
        }

        if (lexer->text[lexer->charIndex] == LXR_OPRND_BRK) {
            lexer->charIndex++;
            SkipSpaces(lexer);
            printf("spaced\n");
        }

        operand[operandIndex] = '\0';
        printf("parsed: %s\n", operand);
        return operand;
    } else if (opcode == OP_MOV) {
        printf("normal register\n");
        char* reg = malloc(12 * sizeof(char));
        int regStrIndex = 0;
        while (isdigit(lexer->text[lexer->charIndex]) || isalpha(lexer->text[lexer->charIndex])) {
            reg[regStrIndex++] = lexer->text[lexer->charIndex++];
        }

        reg[regStrIndex] = '\0';
        return reg;
    }

    if (lexer->text[lexer->charIndex] == LXR_LABEL_START &&
        (opcode == OP_JMP || opcode == OP_JE || opcode == OP_JG || opcode == OP_JGE ||
         opcode == OP_JL || opcode == OP_JLE || opcode == OP_JNE)) {
        printf("Moving to a label\n");
        lexer->charIndex++;
        char* labelName = malloc(MAX_LABEL_LEN * sizeof(char));
        int labelNameIndex = 0;

        while (lexer->text[lexer->charIndex] != LXR_LABEL_END) {
            if (isblank(lexer->text[lexer->charIndex])) {
                printf("is blank.\n");
                break;
            }
            labelName[labelNameIndex] = lexer->text[lexer->charIndex];
            labelNameIndex++;
            lexer->charIndex++;
        }
        labelName[labelNameIndex - 2] = '\0';
        printf("label is: '%s'\n", labelName);
        return labelName;
    }

    // check if operand
    if (!isdigit(lexer->text[lexer->charIndex])) {
        if (opcode != OP_POP && opcode != OP_MOV && opcode != OP_PUSH &&
            lexer->text[lexer->charIndex] != LXR_STR_CHAR) {
            return NULL; // Return NULL for invalid operand
        }
    }

    if (opcode == OP_PUSH && lexer->text[lexer->charIndex] == LXR_STR_CHAR) {
        int stringChars = 1;
        char* string = malloc((STACK_CAPACITY / 4) * sizeof(char));
        int stringIndex = 0;
        string[stringIndex++] = LXR_STR_CHAR; // set first character to string

        lexer->charIndex++;

        // iterate until next string character
        while (lexer->text[lexer->charIndex] != '\0') {
            if (lexer->text[lexer->charIndex] == LXR_STR_CHAR) {
                stringChars++;
                if (stringChars % 2 == 0) {
                    lexer->charIndex++; // Closing quotation mark
                    break;
                }
            } else
                string[stringIndex++] = lexer->text[lexer->charIndex];
            lexer->charIndex++;
        }

        // add null terminator and string at the end
        string[stringIndex++] = LXR_STR_CHAR;
        string[stringIndex] = '\0';

        // strings in pairs
        if (stringChars % 2 != 0) {
            free(string);
            SyntaxError(lexer, "missing quotation mark");
        }
        return string;
    }

    if ((opcode == OP_PUSH && !isdigit(lexer->text[lexer->charIndex]))) {
        char* reg = malloc(12 * sizeof(char));
        int regStrIndex = 0;
        while (!isblank(lexer->text[lexer->charIndex]) && isalpha(lexer->text[lexer->charIndex])) {
            reg[regStrIndex++] = lexer->text[lexer->charIndex++];
        }
        reg[regStrIndex] = '\0';

        return reg;
    }

    if (opcode == OP_POP) {
        char* reg = malloc(12 * sizeof(char));
        int regStrIndex = 0;

        while (isalpha(lexer->text[lexer->charIndex]) || lexer->text[lexer->charIndex] == '8' ||
               lexer->text[lexer->charIndex] == '9') {
            reg[regStrIndex++] = lexer->text[lexer->charIndex++];
        }

        reg[regStrIndex] = '\0';
        return reg;
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

void ParseOperands(Lexer* lexer, Opcode opcode, Operand* operands) {
    if (opcode == OP_POP || opcode == OP_PUSH) {
        // check if theres an operand
        char* operand = ParseOperand(lexer, opcode);
        if (operand == NULL) {
            return; // no operand for pop, we're just deleted it
        }

        if (strlen(operand) == 0) {
            return;
        }

        if (opcode == OP_PUSH && (isdigit(operand[0]) || operand[0] == LXR_STR_CHAR)) {
            ToOperandType(operands, 0, operand);
            CheckOperandSyntax(lexer, opcode, operand);
            return;
        }

        Register reg = GetRegisterFromName(operand);
        if (reg == REG_UNKNOWN)
            SyntaxError(lexer, "invalid register");

        operands[0].data.ptr = (char*)operand;
        operands[0].type = TY_STR;

        return;
    }

    // parse 2 operands maximum
    for (int opIndex = 0; opIndex < OperandsExpected(opcode); opIndex++) {
        if (lexer->text[lexer->charIndex] == LXR_OPRND_BRK) { // ","
            lexer->charIndex++;
            SkipSpaces(lexer);
        }
        // Parse the operand associated with the opcode
        char* operand = ParseOperand(lexer, opcode);
        if (operand == NULL)
            SyntaxError(lexer, "missing operand");

        if (opcode == OP_MOV && operand[0] == LXR_CONSTANT_PREFIX) {
            printf("constant mov\n");
            operands[0].data.ptr = operand;
            operands[0].type = TY_STR;
            printf("- %s\n", (char*)operands[0].data.ptr);
            CheckOperandSyntax(lexer, opcode, operand);
            continue;
        }

        if (opcode == OP_JMP || opcode == OP_JE || opcode == OP_JG || opcode == OP_JGE ||
            opcode == OP_JL || opcode == OP_JLE || opcode == OP_JNE) {
            if (LabelIndex(lexer, operand) != -1) {
                operands[0].data.i64 = LabelIndex(lexer, operand);
                operands[0].type = TY_I64;
                CheckOperandSyntax(lexer, opcode, operand);
            }
            return;
        }

        printf("operand parsed: %s\n", operand);
        ToOperandType(operands, opIndex, operand);

        // Check the syntax of the operand
        CheckOperandSyntax(lexer, opcode, operand);
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

void ToOperandType(Operand* operands, int index, char* operand) {
    if (GetRegisterFromName(operand) != OP_UNKNOWN) {
        // is register
        operands[index].data.i64 = GetRegisterFromName(operand);
        operands[index].type = TY_I64;
        return;
    }

    long asNumber = atol(operand);

    if (operand[0] == LXR_STR_CHAR && operand[strlen(operand) - 1] == LXR_STR_CHAR) {
        operands[index].data.ptr = operand;
        operands[index].type = TY_STR;
    } else {
        operands[index].data.i64 = asNumber;
        operands[index].type = TY_I64;
    }
}

int LabelIndex(Lexer* lexer, char* name) {
    for (int i = 0; i < lexer->numLabels; i++) {
        printf("l: %s, n: %s\n", lexer->labels[i].name, name);
        if (strcmp(lexer->labels[i].name, name) == 0) {
            printf("found label. instructions start at %d\n", lexer->labels[i].index);
            return lexer->labels[i].index;
        }
    }
    return -1;
}

void SkipSpaces(Lexer* lexer) {
    // skip spaces, go until not a space
    while (isblank(lexer->text[lexer->charIndex])) {
        // value for token
        // get numeric value. get all digits
        lexer->charIndex++;
    }
}

char CurrentChar(Lexer* lexer) { return lexer->text[lexer->charIndex]; }

BOOL UniqueLabelName(Label* label, Lexer* lexer) {
    for (unsigned int i = 0; i < lexer->numLabels; i++) {
        if (strcmp(label->name, lexer->labels[i].name) == 0)
            return FALSE;
    }
    return TRUE;
}

#ifndef USING_ARDUINO
Lexer ParseTokens(char* path)
#elif defined(USING_ARDUINO)
Lexer ParseTokens(char* text) 
#endif
{
    // Open file and load its contents
    long tl = 0;
    #ifndef USING_ARDUINO
        char* text = ReadFromFile(path, &tl);
    #elif defined(USING_ARDUINO)
        char* path = "none";
    #endif

    // Create lexxer struct from known variables
    Lexer lexer = {
        .state = PARSE, .lineNumber = 1, .filePath = path, .text = text, .textLength = tl};

    char keyword[MAX_KEYWORD_LEN] = {0};
    long index = 0;

    Label currLabel = {0};

    while (lexer.charIndex <= lexer.textLength && lexer.numTokens <= MAX_PROGRAM_SIZE) {
        if (lexer.state == SKIP_SPACES) {
            if (isblank(lexer.text[lexer.charIndex])) {
                lexer.charIndex++;
                continue;
            }
            lexer.state = PARSE;
            // printf("Parse. Used to skip spaces.\n");
        }

        if (lexer.text[lexer.charIndex] == LXR_LABEL_START &&
            (lexer.state != PARSE_KWD || lexer.state != PARSE_OPND)) {
            if (lexer.state == PARSE_LABEL) // theres another underscore?
                SyntaxError(&lexer, "unrecognized label token");

            // printf("Label");
            memset(&currLabel, 0, sizeof(Label));
            currLabel.index = lexer.numTokens;
            lexer.state = PARSE_LABEL;
            lexer.charIndex++;
        }

        // is comment
        if (lexer.text[lexer.charIndex] == LXR_COMMENT && lexer.state != SKIP_LINE) {
            // printf("Is comment. '%c' '%s'\n", lexer.text[lexer.charIndex],
            // keyword);

            lexer.state = SKIP_LINE;
            lexer.charIndex++;
            continue;
        }

        // skipping line
        if (lexer.state == SKIP_LINE && lexer.text[lexer.charIndex] != '\n') {
            lexer.charIndex++;
            continue;
        } else if (lexer.state == SKIP_LINE && lexer.text[lexer.charIndex] == '\n') {
            lexer.charIndex++;
            lexer.lineNumber++;
            lexer.state = PARSE;
            // printf("Stopped skipping line.\n");
            continue;
        }

        // parse a label
        if (lexer.state == PARSE_LABEL) {
            if (lexer.text[lexer.charIndex] == LXR_LABEL_END) {
                // reached the end of the label
                if (currLabel.nameLen == 0)
                    SyntaxError(&lexer, "unnamed label");

                if (UniqueLabelName(&currLabel, &lexer) == FALSE)
                    SyntaxError(&lexer, "duplicate label name");

                lexer.labels[lexer.numLabels++] = currLabel;
                printf("%ld _%s:\n", currLabel.index, currLabel.name);
                lexer.state = SKIP_LINE;
                continue;
            } else if (isalpha(lexer.text[lexer.charIndex])) {
                // name of the label
                currLabel.name[currLabel.nameLen++] = lexer.text[lexer.charIndex];
                lexer.charIndex++;
                continue;
            }
        }

        // detect unknonwn characters
        if (lexer.charIndex < lexer.textLength && !isalpha(lexer.text[lexer.charIndex]) &&
            !isspace(lexer.text[lexer.charIndex])) {
            if (lexer.text[lexer.charIndex] != LXR_STR_CHAR) {
                char buff[25] = {0};
                snprintf(buff, 25, "unknown character '%c'", lexer.text[lexer.charIndex]);
                SyntaxError(&lexer, buff);
            }
        }

        // We are in general parsing state
        if (lexer.state == PARSE) {
            lexer.state = PARSE_KWD;
            index = 0;
            memset(keyword, 0, MAX_KEYWORD_LEN);
        }

        // Parse a keyword
        if (lexer.state == PARSE_KWD) {
            if (lexer.charIndex < lexer.textLength &&
                (isalpha(lexer.text[lexer.charIndex]) &&
                 lexer.text[lexer.charIndex] != LXR_COMMENT)) {

                keyword[index++] = lexer.text[lexer.charIndex++];

                if (OpcodeFromKeyword(keyword) == OP_UNKNOWN &&
                    (!isalpha(lexer.text[lexer.charIndex + 1]) ||
                     !isdigit(lexer.text[lexer.charIndex + 1])))
                    continue;
            }

            if (index == 0) {
                lexer.charIndex++;
                continue;
            }

            keyword[index] = '\0';
            lexer.state = PARSE;
        }

        printf("keyword: %s\n", keyword);
        // get opcode from keyword as an Opcode enum
        Opcode opcode = OpcodeFromKeyword(keyword);
        if (opcode == OP_UNKNOWN) {
            char buff[MAX_KEYWORD_LEN + 20] = {0};
            snprintf(buff, MAX_KEYWORD_LEN + 20, "unknown opcode '%s'", keyword);
            SyntaxError(&lexer, buff);
        }

        SkipSpaces(&lexer); // skip any spaces between opcode keyword and operands

        Operand operands[2] = {0};
        ParseOperands(&lexer, opcode, operands);

        // Create token from keyword, opcode, and operands
        Token token = NewToken(opcode, strdup(keyword), operands, &lexer);
        lexer.tokens[lexer.numTokens++] = token; // append token to array of tokens
    }

    printf("Parsed %d instructions.\n", lexer.numTokens);

    free(text);
    return lexer;
}