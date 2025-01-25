/// Macros and Instruction Macros
///
/// Includes macros to make creating Instruction structs
/// neater. Instead of having to do {.operation = OP_PUSH, .data.value = 3}
/// everytime you have to push a value, you can simply use an instruction macro.

#define USING_ARDUINO // comment this out if not using arduino

#ifdef USING_ARDUINO
#define STACK_CAPACITY 15
#define MEMORY_CAPACITY 25
#define MAX_PROGRAM_SIZE 15
#define MAX_KEYWORD_LEN 12 // keywords should NOT exceed this length
#define MAX_OPERAND_LEN 10 // worst case scenario you have two LLONG_MAX
#define MAX_STRING_LEN 10
#define MAX_LABELS 5
#define MAX_LABEL_LEN 8

// ripped from the internet
// registers for the arduino to control pin states
#define DDRB                                                                   \
  (*(volatile unsigned char *)0x24) // Data Direction Register for Port B
#define DDRC                                                                   \
  (*(volatile unsigned char *)0x27) // Data Direction Register for Port C
#define DDRD                                                                   \
  (*(volatile unsigned char *)0x2A) // Data Direction Register for Port D
#define DRPORTB (*(volatile unsigned char *)0x25) // Port B Data Register
#define DRPORTC (*(volatile unsigned char *)0x28) // Port C Data Register
#define DRPORTD (*(volatile unsigned char *)0x2B) // Port D Data Register
#define INPB (*(volatile unsigned char *)0x23)
#define INPC (*(volatile unsigned char *)0x26)
#define INPD (*(volatile unsigned char *)0x29)
#elif !defined(                                                                \
    USING_ARDUINO) // if not using arduino, the max sizes can be a bit bigger
#define STACK_CAPACITY 2048
#define MEMORY_CAPACITY 2048
#define MAX_PROGRAM_SIZE 2048
#define MAX_KEYWORD_LEN 35 // key
#define MAX_OPERAND_LEN 50
#define MAX_STRING_LEN 256
#define MAX_LABELS 100
#define MAX_LABEL_LEN 15
#endif

#define LXR_MAX_LINE_LEN                                                       \
  MAX_KEYWORD_LEN + MAX_OPERAND_LEN // maximum length a line can be lexer

/// Syntax for lexer
#define LXR_COMMENT ';' // a command in the language, like c has // for comment
#define LXR_OPRND_BRK ','   // character that seperates operands
#define LXR_REG_PREFIX 'r'  // register prefix for operations like mov
#define LXR_STR_CHAR '"'    // start and end of a string
#define LXR_LABEL_START '_' // start of a label
#define LXR_LABEL_END ':'   // end of a label
#define LABEL_ENTRY_PNT "start"
#define LXR_CONSTANT_PREFIX '$'
#define LXR_ESCAPE_CHAR '\\'

// types
typedef int BOOL;

#define FALSE 0
#define TRUE 1

/// Insturction macros
#define INST_NOP() {.operation = OP_NOP}

#define INST_PUSH(a) {.operation = OP_PUSH, .data.value = a}
#define INST_POP() {.operation = OP_POP}
#define INST_MOV(source, destination)                                          \
  {.operation = OP_MOV,                                                        \
   .data.registers.src = source,                                               \
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