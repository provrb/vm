/// Instructions Header File
///
/// Functions to handle operations
/// Opcode enum definition
/// Instruction struct definiton..

#ifndef INST_H
#define INST_H

#include "machine.h"

/// @brief Enum represnting assembly instructions
///
/// Depending on an opcode, perform different functions
typedef enum {
    OP_NOP = 0,
    OP_PUSH,
    OP_POP,
    OP_MOV,
    OP_SWAP,
    
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    
    OP_NEG,

    OP_ANDB,
    OP_ORB,
    OP_NOTB,
    OP_XORB,
    OP_SHL,
    OP_SHR,

    OP_DUP,
    OP_CLR,
    OP_SIZE,
    OP_PRNT
} Opcode;

/// A struct that represents an assembly instructions
///
/// operation: opcode representing an assembly operation
/// data: union containing either a value or a source and destination register to use
typedef struct {
    Opcode operation;
    union {
        int value;
        struct {
            int src;
            int dest;
        } registers;
    } data;
} Instruction;

void Move(Machine* machine, int src, int dest);
void Push(Machine* machine, int value);
int Pop(Machine* machine);
void ClearStack(Machine* machine);
void PrintStack(Machine* machine);
void RunInstructions(Machine* machine);

#endif