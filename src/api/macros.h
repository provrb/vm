/// Macros and Instruction Macros
///
/// Includes macros to make creating Instruction structs
/// neater. Instead of having to do {.operation = OP_PUSH, .data.value = 3}
/// everytime you have to push a value, you can simply use an instruction macro.

/// Size of the program array containing opcodes
#define PROGRAM_SIZE sizeof(program) / sizeof(program[0])

/// Insturction macros
#define INST_PUSH(a)   {.operation = OP_PUSH, .data.value = a}
#define INST_POP()     {.operation = OP_POP}
#define INST_MOV(a, b) {.operation = OP_MOV, .data.registers.src = a, .data.registers.dest = b}
#define INST_PRNT()    {.operation = OP_PRNT}

#define INST_ADD()     {.operation = OP_ADD}
#define INST_DIV()     {.operation = OP_DIV}
#define INST_MOD()     {.operation = OP_MOD}
#define INST_SUB()     {.operation = OP_SUB}
#define INST_MUL()     {.operation = OP_MUL}

#define INST_CLR()     {.operation = OP_CLR}
#define INST_SIZE()    {.operation = OP_SIZE}
#define INST_NEG()     {.operation = OP_NEG}

#define INST_OR()      {.operation = OP_ORB}
#define INST_XOR()     {.operation = OP_XORB}
#define INST_AND()     {.operation = OP_ANDB}
#define INST_NOT()     {.operation = OP_NOTB}