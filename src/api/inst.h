/// Instructions Header File
///
/// Functions to handle operations
/// Opcode enum definition
/// Instruction struct definiton..

#ifndef INST_H
#define INST_H

#include "machine.h" // Machine struct definition

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
        const int a = machine->stack[machine->stackSize - 2];                                      \
        const int b = machine->stack[machine->stackSize - 1];                                      \
                                                                                                   \
        if ((a) operator(b)) {                                                                     \
            JumpTo(machine, addr);                                                                 \
            *res = TRUE;                                                                           \
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

/// @brief Decribe the state of an instructions
/// Whether or not it has be ran or not, useful for jump
typedef enum {
    IS_PENDING = 3,
    IS_EXECUTED = 5,
} InstState;

/// @brief A struct that represents an assembly instructions
///
/// @param operation: opcode representing an assembly operation
/// @param data: union containing either a value or a source and destination
/// register to use
typedef struct {
    Opcode operation;
    InstState state; // TRUE or FALSE
    union {
        int value;
        struct {
            int src;
            int dest;
        } registers;
    } data;
} Instruction;

/// @brief Move the value on stack at index 'src' to index 'dest'
/// @param machine - machine to perform move operation on
/// @param src - index of value to move
/// @param dest - index to move src
void Move(Machine* machine, int src, int dest);

/// @brief Push a value 'value' to the machines stack
/// @param machine - machine to append 'value' to its stack
/// @param value - number to append to machines stack
void Push(Machine* machine, int value);

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