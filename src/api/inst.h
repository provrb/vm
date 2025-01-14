/// Instructions Header File
///
/// Functions to handle operations
/// Opcode enum definition
/// Instruction struct definiton..

/// @brief Read and run functions in program based off opcodes
void RunInstructions();

/// Functions to handle operatons
void Move(int src, int dest);
void Push(int value);
int  Pop();
void ClearStack();
void PrintStack();

/// @brief Enum represnting assembly instructions
///
/// Depending on an opcode, perform different functions
typedef enum {
    OP_PUSH,
    OP_POP,
    OP_MOV,
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

