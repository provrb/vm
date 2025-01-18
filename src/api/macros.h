/// Macros and Instruction Macros
///
/// Includes macros to make creating Instruction structs
/// neater. Instead of having to do {.operation = OP_PUSH, .data.value = 3}
/// everytime you have to push a value, you can simply use an instruction macro.

#define STACK_CAPACITY 1024
#define MAX_PROGRAM_SIZE 1024

// booleans
#define FALSE 0
#define TRUE 1

/// Insturction macros

#define INST_NOP() {.operation = OP_NOP}

#define INST_PUSH(a) {.operation = OP_PUSH, .data.value = a}
#define INST_POP() {.operation = OP_POP}
#define INST_MOV(source, destination)                                          \
    {.operation = OP_MOV,                                                      \
     .data.registers.src = source,                                             \
     .data.registers.dest = destination}
#define INST_PRNT() {.operation = OP_PRNT}
#define INST_SWAP() {.operation = OP_SWAP}

#define INST_ADD() {.operation = OP_ADD}
#define INST_DIV() {.operation = OP_DIV}
#define INST_MOD() {.operation = OP_MOD}
#define INST_SUB() {.operation = OP_SUB}
#define INST_MUL() {.operation = OP_MUL}

#define INST_JMP(to) {.operation = OP_JMP, .data.value = to}
#define INST_JNE(to) {.operation = OP_JNE, .data.value = to}
#define INST_JE(to) {.operation = OP_JE, .data.value = to}
#define INST_JG(to) {.operation = OP_JG, .data.value = to}
#define INST_JGE(to) {.operation = OP_JGE, .data.value = to}
#define INST_JL(to) {.operation = OP_JL, .data.value = to}
#define INST_JLE(to) {.operation = OP_JLE, .data.value = to}

#define INST_DUP() {.operation = OP_DUP}
#define INST_CLR() {.operation = OP_CLR}
#define INST_SIZE() {.operation = OP_SIZE}
#define INST_NEG() {.operation = OP_NEG}

#define INST_ORB() {.operation = OP_ORB}
#define INST_XORB() {.operation = OP_XORB}
#define INST_ANDB() {.operation = OP_ANDB}
#define INST_NOTB() {.operation = OP_NOTB}
#define INST_SHL(a) {.operation = OP_SHL, .value = a}
#define INST_SHR(a) {.operation = OP_SHR, .value = a}