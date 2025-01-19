/// Instructions Header File
///
/// Functions to handle operations
/// Opcode enum definition
/// Instruction struct definiton..

#ifndef INST_H
#define INST_H

#include "macros.h"

/// Compare last two values in stack based off operator
/// if the expression evaluates to true, jump to 'addr' and set res to TRUE
///
/// @param operator - logical operator (>=, ==, <=, <, >)
/// @param machine: Machine* - machine containing a stack
/// @param addr: int -  index to jump to if expression is true
/// @param res: int* - int* to receive the result of expression
#define JUMP_IF(operator, machine, addr, res)                                                      \
    {                                                                                              \
        *res = FALSE;                                                                              \
                                                                                                   \
        if (machine->stackSize - 2 < 0) {                                                          \
            fprintf(stderr, "Not enough values on stack for comparison.\n");                       \
            exit(1);                                                                               \
        }                                                                                          \
                                                                                                   \
        const Data a = machine->stack[machine->stackSize - 2];                                     \
        const Data b = machine->stack[machine->stackSize - 1];                                     \
                                                                                                   \
        if ((a.type == TY_U64 || a.type == TY_I64) && (b.type == TY_U64 || b.type == TY_I64)) {    \
            if ((a.data.i64) operator(b.data.i64)) {                                               \
                JumpTo(machine, addr);                                                             \
                *res = TRUE;                                                                       \
            }                                                                                      \
        }                                                                                          \
    }

/// @brief Enum represnting assembly instructions
///
/// Depending on an opcode, perform different functions
typedef enum {
    OP_UNKNOWN = -1,
    OP_NOP = 0,
    OP_PUSH,
    OP_POP,
    OP_MOV,
    OP_SWAP,

    OP_JMP,
    OP_JNE,
    OP_JE,
    OP_JG,
    OP_JGE,
    OP_JL,
    OP_JLE,

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

typedef enum {
    TY_U64,
    TY_I64,
    TY_BYTE,
    TY_STR,
    TY_ANY,
} DataType;

typedef union {
    unsigned long u64;
    long i64;
    char byte;
    void* ptr;
} Cell;

typedef struct {
    Cell data;
    DataType type;
} Data;

typedef Data Operand;

/// @brief A struct that represents an assembly instructions
///
/// @param operation: opcode representing an assembly operation
/// @param data: union containing either a value or a source and destination
/// register to use
typedef struct {
    Opcode operation;
    union {
        Operand value;
        struct {
            int src;
            int dest;
        } registers;
    } data;
} Instruction;

typedef struct {
    Data stack[STACK_CAPACITY];
    int stackSize;

    Instruction* program; // this should be an array of Instruction
    int programSize;
    int ip; // instruction
} Machine;

Data DATA_USING_I64(long val);

Data DATA_USING_U64(unsigned long val);

Data DATA_USING_STR(char* val);

Data DATA_USING_PTR(void* val);

/// @brief Move the value on stack at index 'src' to index 'dest'
/// @param machine - machine to perform move operation on
/// @param src - index of value to move
/// @param dest - index to move src
void Move(Machine* machine, int src, int dest);

/// @brief Push a value 'value' to the machines stack
/// @param machine - machine to append 'value' to its stack
/// @param value - number to append to machines stack
void Push(Machine* machine, Data value);

/// @brief Remove the last element on the stack
/// @param machine - machine to perform the operation on
/// @return - the removed element
int Pop(Machine* machine);

/// @brief Remove all elements from the stack
/// @param machine - machine to perform the operation on
void ClearStack(Machine* machine);

/// @brief Output all elements in the stack
/// @param machine - machine to perform the operation on
void PrintStack(Machine* machine);

/// @brief Run the instruction located at machine->program[machine->ip]
/// @param machine - machine to perform the operation on
void RunInstructions(Machine* machine);

Instruction* ReadProgramFromFile(Machine* machine, char* path);
void DumpProgramToFile(Machine* inst, char* filePath);

#endif