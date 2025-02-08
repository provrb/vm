/// Instructions Header File
///
/// Functions to handle operations
/// Opcode enum definition
/// Instruction struct definiton..

#ifndef INST_H
#define INST_H

#include "macros.h"

#include <stdint.h>

#define ARITHMETIC(operator, inst, machine, op)                                                    \
    long a = -1;                                                                                   \
    double af = -1;                                                                                \
    BOOL fp = FALSE;                                                                               \
                                                                                                   \
    if (!strcmp(GetRegisterName(inst.data.value.data.i64), "unknown")) {                           \
        RemoveChar((char*)inst.data.value.data.ptr, LXR_CONSTANT_PREFIX);                          \
        inst.data.value.type = IsFloat((char*)inst.data.value.data.ptr) ? TY_F64 : TY_I64;         \
                                                                                                   \
        if (inst.data.value.type == TY_F64)                                                        \
            fp = TRUE;                                                                             \
        else                                                                                       \
            fp = FALSE;                                                                            \
        if (fp == FALSE)                                                                           \
            a = atol((char*)inst.data.value.data.ptr);                                             \
        else                                                                                       \
            af = strtod((char*)inst.data.value.data.ptr, NULL);                                    \
        if ((op == '/' || op == '%') && (a == 0 || af == 0))                                       \
            RuntimeError("divide by zero error.");                                                 \
    }                                                                                              \
                                                                                                   \
    Data* dest = &machine->memory[inst.data.registers.dest];                                       \
    printf("a: %d af: %f, dest: %d\n", a, af, dest->data.i64);                                     \
    if (dest->type == TY_I64)                                                                      \
        *dest = (fp == FALSE) ? DATA_USING_I64(dest->data.i64 operator a)                          \
                              : DATA_USING_F64(dest->data.i64 operator af);                        \
    else                                                                                           \
        *dest = DATA_USING_F64((fp == FALSE) ? (dest->data.f64 operator a)                         \
                                             : (dest->data.f64 operator af));

typedef enum { PORT_B, PORT_C, PORT_D } ArduinoPort;

// General purpose register indexes
// for accessing memory in a Machine
typedef enum {
    REG_UNKNOWN = -1,
    REG_NONE = 0x0,
    REG_RAX = 0x1,
    REG_RBX = 0x2,
    REG_RCX = 0x3,
    REG_RDX = 0x4,
    REG_R8 = 0x5,
    REG_R9 = 0x6,
    REG_R10 = 0x7,
    REG_R11 = 0x8,
    REG_R12 = 0x9,
    REG_R13 = 0xA,
    REG_R14 = 0xB,
    REG_R15 = 0xC,
    REG_EP = 0xBB,
    REG_CP = 0x83, // ptr where a jmp was called to resume instructions afterwards
} Register;

typedef struct {
    const char* name;
    Register reg;
} RegisterMap;

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

    OP_CALL,
    OP_RET,

    OP_CMP,

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
    OP_OREB, // or equals
    OP_NOTB,
    OP_XORB,
    OP_SHL,
    OP_SHR,

    OP_DUP,
    OP_CLR,
    OP_SIZE,
    OP_PRNT,

    OP_WRITE,   // write to stdout or stderr or write pin for arduino
    OP_READ,    // stdin or read pin for arduino
    OP_ANWRITE, // arduino only analog write

    OP_SYSCALL,

    OP_EXIT,
} Opcode;

typedef enum {
    SYS_EXEC = 1,
    SYS_ALLOC,
    SYS_FREE,
    SYS_REALLOC,
    SYS_PROTECT,
    SYS_ENV,
    SYS_SLEEP,
    SYS_CYCLES,
    SYS_UNKNOWN,
} Syscall;

typedef enum {
    TY_EMPTY,
    TY_U64,
    TY_I64,
    TY_F64,
    TY_BYTE,
    TY_STR,
    TY_REG_NAME,
    TY_ANY,
} DataType;

typedef enum {
    FILE_STDIN = 1,
    FILE_STDERR,
    FILE_STDOUT,
    FILE_INOPIN, // for arduino
} FileDescriptor;

typedef union {
    unsigned long u64;
    long i64;
    double f64;
    char byte;
    void* ptr;
} DataCell;

typedef struct {
    DataCell data;
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
    struct {
        Operand value;
        struct {
            unsigned int src;
            unsigned int dest;
        } registers;
    } data;
} __attribute__((packed)) Instruction;

/// Represents a label
/// e.g _start:
/// always starts with 'LXR_LABEL_
typedef struct {
    char name[MAX_LABEL_LEN];
    unsigned short nameLen;
    long index;
} Label;

typedef struct {
    // Arrays simulating cpu memory and a stack
    Data stack[STACK_CAPACITY];
    Data memory[MEMORY_CAPACITY];
    Label labels[MAX_LABELS]; // labels parsed from the lexer

    // Program
    Instruction* program;

    uint32_t stackSize;
    uint32_t memorySize;
    uint32_t numLabels;
    uint32_t programSize;
    uint32_t cycles; // instructions ran
    uint32_t ip;     // instruction
    uint32_t rp;     // return pointer. index to set ip to after we are finished in a label

    // flags
    uint8_t EFLAGS;

    // has executed the first instruction
    BOOL started;
} Machine;

// Create Data structures using different available types
Data DATA_USING_F64(double val);
Data DATA_USING_I64(long val);
Data DATA_USING_U64(unsigned long val);
Data DATA_USING_STR(char* val);

#ifdef USING_ARDUINO
/// @brief Get the port for a pin
ArduinoPort PinPort(int pin);

/// @brief Get the bit index for a pin
int PinBit(int pin);

unsigned char PortPinRegister(int pin);

unsigned char DataDirPinRegister(int pin);
#endif

/// @brief Move the value on stack at index 'src' to index 'dest'
/// @param machine - machine to perform move operation on
/// @param src - index of value to move
/// @param dest - index to move src
void Move(Machine* machine, Operand data, int dest);

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

/// @brief Print the contents of each register in machines memory
/// @param machine - machine to print register contents
void PrintRegisterContents(Machine* machine);

const char* GetRegisterName(Register reg);
Register GetRegisterFromName(const char* name);

Instruction* ReadProgramFromFile(Machine* machine, char* path);
void DumpProgramToFile(Machine* inst, char* filePath);

#endif