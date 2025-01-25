#include "inst.h"
#include "macros.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef USING_ARDUINO
static const RegisterMap registerMap[] = {
    {"rax", REG_RAX}, {"rbx", REG_RBX}, {"rcx", REG_RCX}, {"rdx", REG_RDX}, {"r8", REG_R8},
    {"r9", REG_R9},   {"r10", REG_R10}, {"r11", REG_R11}, {"r12", REG_R12}, {"r13", REG_R13},
    {"r14", REG_R14}, {"r15", REG_R15}, {"ep", REG_EP},   {"cp", REG_CP},   {"none", REG_UNKNOWN}};

const char* GetRegisterName(Register reg) {
    for (int i = 0; registerMap[i].reg != REG_UNKNOWN; i++)
        if (registerMap[i].reg == reg)
            return registerMap[i].name;

    return "unknown";
}

Register GetRegisterFromName(const char* name) {
    for (int i = 0; registerMap[i].reg != REG_UNKNOWN; i++)
        if (strcmp(registerMap[i].name, name) == 0)
            return registerMap[i].reg;

    return REG_UNKNOWN;
}

#elif defined(USING_ARDUINO)

Register GetRegisterFromName(const char* keyword) {
    if (strcmp(keyword, "rax") == 0)
        return REG_RAX;
    else if (strcmp(keyword, "rbx") == 0)
        return REG_RBX;
    else if (strcmp(keyword, "rcx") == 0)
        return REG_RCX;
    else if (strcmp(keyword, "rdx") == 0)
        return REG_RDX;
    else if (strcmp(keyword, "r8") == 0)
        return REG_R8;
    else if (strcmp(keyword, "r9") == 0)
        return REG_R9;
    else if (strcmp(keyword, "r10") == 0)
        return REG_R10;
    else if (strcmp(keyword, "r11") == 0)
        return REG_R11;
    else if (strcmp(keyword, "r12") == 0)
        return REG_R12;
    else if (strcmp(keyword, "r13") == 0)
        return REG_R13;
    else if (strcmp(keyword, "r14") == 0)
        return REG_R14;
    else if (strcmp(keyword, "r15") == 0)
        return REG_R15;
    else if (strcmp(keyword, "ep") == 0)
        return REG_EP;
    else if (strcmp(keyword, "cp") == 0)
        return REG_CP;
    else if (strcmp(keyword, "none") == 0)
        return REG_UNKNOWN;
    else
        return REG_UNKNOWN;
}

const char* GetRegisterName(Register reg) {
    switch (reg) {
    case REG_RAX:
        return "rax";
    case REG_RBX:
        return "rbx";
    case REG_RCX:
        return "rcx";
    case REG_RDX:
        return "rdx";
    case REG_R8:
        return "r8";
    case REG_R9:
        return "r9";
    case REG_R10:
        return "r10";
    case REG_R11:
        return "r11";
    case REG_R12:
        return "r12";
    case REG_R13:
        return "r13";
    case REG_R14:
        return "r14";
    case REG_R15:
        return "r15";
    case REG_EP:
        return "ep";
    case REG_CP:
        return "cp";
    case REG_UNKNOWN:
        return "unknown";
    default:
        return "none";
    }
}

#endif

Data DATA_USING_I64(long val) {
    Data d = {.data.i64 = val, .type = TY_I64};
    return d;
}

Data DATA_USING_U64(unsigned long val) {
    Data d = {.data.u64 = val, .type = TY_U64};
    return d;
}

Data DATA_USING_STR(char* val) {
    Data d = {.data.ptr = val, .type = TY_STR};
    return d;
}

Data DATA_USING_PTR(void* val) {
    Data d = {.data.ptr = val, .type = TY_ANY};
    return d;
}

void RemoveChar(char* str, char toRemove) {
    int i, j = 0;
    int length = strlen(str);

    for (i = 0; i < length; i++) {
        if (str[i] != toRemove) {
            str[j++] = str[i];
        }
    }
    str[j] = '\0';
}

void Move(Machine* machine, Operand data, int dest) {
    const char* regDest = GetRegisterName(dest);
    if (strcmp(regDest, "unknown") == 0) {
        fprintf(stderr, "Invalid destination register. Aborted.\n");
        exit(1);
    }

    if (strcmp(GetRegisterName(data.data.i64), "unknown") == 0) {
        printf("Not a source register. Taking as a constant %s\n", (char*)data.data.ptr);
        RemoveChar((char*)data.data.ptr, LXR_CONSTANT_PREFIX);
        machine->memory[dest] = DATA_USING_I64(atol((char*)data.data.ptr));
        return;
    }

    machine->memory[dest] = machine->memory[data.data.i64];
}

void Push(Machine* machine, Data value) {
    if (machine->stackSize >= STACK_CAPACITY) {
        fprintf(stderr, "Stack overflow when trying to push value to stack. Aborted.\n");
        exit(1);
    }

    machine->stack[machine->stackSize++] = value;
}

int Pop(Machine* machine) {
    if (machine->stackSize <= 0) {
        fprintf(stderr, "Stack underflow when trying to pop from stack. Aborted.\n");
        exit(1);
    }

    return machine->stack[--machine->stackSize].data.i64;
}

void ClearStack(Machine* machine) {
    if (machine->stackSize == 0)
        return;

    int tempSize = machine->stackSize; // need to make a temp copy because
                                       // stackSize changes as we iterate
    for (int i = 0; i < tempSize; i++) {
        Pop(machine);
    }
}

void PrintStack(Machine* machine) {
    printf("--- Stack Start ---\n");
    for (int i = machine->stackSize - 1; i >= 0; i--) {
        Data x = machine->stack[i];
        if (x.type == TY_STR)
            printf("%s\n", (char*)x.data.ptr);
        else if (x.type == TY_I64 || x.type == TY_U64)
            printf("%ld\n", machine->stack[i].data.i64);
    }
    printf("--- Stack End   ---\n");
}

void JumpTo(Machine* machine, int dest) {
    if (dest > machine->programSize || dest < 0) {
        return;
    }

    machine->ep = dest;
    machine->rp = machine->ip;
    machine->ip = dest;
}

void DumpProgramToFile(Machine* machine, char* filePath) {
    FILE* file = fopen(filePath, "wb");
    if (file == NULL) {
        fprintf(stderr, "Error opening file. Path: %s\n", filePath);
        exit(1);
    }

    fwrite(machine->program, sizeof(Instruction), machine->programSize, file);
    fclose(file);
}

void PrintRegisterContents(Machine* machine) {
    printf("Memory Layout\n");

    for (int i = REG_RAX; i < REG_R15 + 1; i++) {
        Data data = machine->memory[i];
        printf("%-4s: ", GetRegisterName(i));

        switch (data.type) {
        case TY_STR:
            printf("\"%2s\"", (char*)data.data.ptr);
            break;
        case TY_I64:
            printf("%5ld (i64)", data.data.i64);
            break;
        case TY_U64:
            printf("%5ld (u64)", data.data.u64);
            break;
        default:
            printf("empty");
            break;
        }

        printf("\n");
    }
}

Instruction* ReadProgramFromFile(Machine* machine, char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error opening file. Path: %s\n", path);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    Instruction* insts = malloc(length);
    if (insts == NULL) {
        fprintf(stderr, "Buffer allocation error for file contents.\n");
        exit(1);
    }

    fread(insts, sizeof(Instruction), length / sizeof(Instruction), file);
    fclose(file);

    machine->program = insts;
    machine->programSize = length / sizeof(Instruction);

    return insts;
}

void RuntimeError(char* msg) {
    fprintf(stderr, "runtime error. %s\n", msg);
    exit(1);
}

int GetEntryPoint(Machine* machine) {
    for (int i = 0; i < machine->numLabels; i++) {
        if (strcmp(LABEL_ENTRY_PNT, machine->labels[i].name) == 0)
            return machine->labels[i].index;
    }

    RuntimeError("no entry point"); // no entry point
}

int EndOfLabel(Machine* machine) {
    int nextLabelIndex = -1;
    int closestIndex = 2147483647;

    for (int i = 0; i < machine->numLabels; i++) {
        if (machine->labels[i].index == machine->ep) {
            nextLabelIndex = machine->labels[i + 1].index - 1;
            break;
        }
    }

    return nextLabelIndex;
}

void Zero(Data* memory, Register reg) { memory[reg] = DATA_USING_I64(0); }

#ifdef USING_ARDUINO
/// @brief Get the port for a pin
ArduinoPort PinPort(int pin) {
    if (pin >= 0 && pin <= 7)
        return PORT_D;
    else if (pin >= 8 && pin <= 13)
        return PORT_B;
    else if (pin >= 14 && pin <= 19)
        return PORT_C;
    return -1;
}

unsigned char DataDirPinRegister(int pin) {
    switch (PinPort(pin)) {
    case PORT_B:
        return DDRB;
    case PORT_C:
        return DDRC;
    case PORT_D:
        return DDRD;
    }
    return 492838;
}

unsigned char PortPinRegister(int pin) {
    switch (PinPort(pin)) {
    case PORT_B:
        return DRPORTB;
    case PORT_C:
        return DRPORTC;
    case PORT_D:
        return DRPORTD;
    }
    return 0;
}

/// @brief Get the bit index for a pin
int PinBit(int pin) {
    ArduinoPort port = PinPort(pin);

    switch (port) {
    case PORT_B:
        return pin - 8;
    case PORT_C:
        return pin - 14;
    case PORT_D:
        return pin;
    }

    return -1;
}
#endif

void OutputString(char* string, FileDescriptor fd) {
    if (string[0] != LXR_STR_CHAR || string[strlen(string) - 1] != LXR_STR_CHAR)
        RuntimeError("invalid string");

    // remnove first quote
    for (size_t i = 0; i < strlen(string); i++)
        string[i] = string[i + 1];

    if (fd != FILE_STDOUT && fd != FILE_STDERR)
        RuntimeError("invalid file descriptor");

    FILE* stream = (fd == FILE_STDOUT) ? stdout : stderr;

    // remove last quote
    string[strlen(string) - 1] = '\0';

    for (size_t i = 0; i < strlen(string); i++) {
        char c = string[i];

        if (c == '\\' && i + 1 < strlen(string)) {
            i++;
            c = string[i];
            switch (c) {
            case 'n':
                fprintf(stream, "\n");
                break;
            case '\\':
                fprintf(stream, "\\");
                break;
            case '\'':
                fprintf(stream, "\'");
                break;
            case '\"':
                fprintf(stream, "\"");
                break;
            case 't':
                fprintf(stream, "\t");
                break;
            case 'r':
                fprintf(stream, "\r");
                break;
            case 'b':
                fprintf(stream, "\b");
                break;
            case 'f':
                fprintf(stream, "\f");
                break;
            case 'a':
                fprintf(stream, "\a");
                break;
            case 'v':
                fprintf(stream, "\v");
                break;
            default:
                fprintf(stream, "\\%c", c);
            }

            continue;
        }

        fprintf(stream, "%c", c);
    }
}

void RunInstructions(Machine* machine) {
    long end = EndOfLabel(machine);
    if (end != -1 && machine->ip > end && machine->rp != -1) {
        machine->ip = machine->rp + 1;
        machine->rp = -1;
        machine->ep = -1;
        RunInstructions(machine);
        return;
    }

    if (machine->ip == 0 && machine->started == FALSE) {
        machine->ip = GetEntryPoint(machine);
        machine->started = TRUE;
    }

    if (machine->ip >= machine->programSize)
        return;

    int jump = FALSE; // if inst.operation is a successful jump
    Instruction inst = ((Instruction*)machine->program)[machine->ip];

    switch (inst.operation) {
    case OP_READ: {
        unsigned int fd = Pop(machine);
        if (fd == FILE_INOPIN) {
#ifndef USING_ARDUINO
            RuntimeError("not implemented");
#endif

            // read input from pin

        } else if (fd == FILE_STDIN) {
            // read input from stdin
            char buffer[MAX_STRING_LEN] = {0};
            buffer[0] = LXR_STR_CHAR;
            if (fgets(buffer + 1, MAX_STRING_LEN, stdin) == NULL)
                break;

            RemoveChar(buffer, '\n');
            buffer[strlen(buffer)] = LXR_STR_CHAR;

            Push(machine, DATA_USING_STR(buffer));
        }

        break;
    }
    case OP_WRITE: {
        unsigned int fd = Pop(machine);
        int toWrite = Pop(machine);
        if (fd == FILE_STDOUT || fd == FILE_STDERR) {
            char* asString = (char*)toWrite; // purposly seg fault if not a string
            OutputString(asString, fd);
        } else if (fd == FILE_INOPIN) {
#ifndef USING_ARDUINO
            RuntimeError("not implemented");
#else
            int state = Pop(machine);
            if (state != 0 && state != 1)
                RuntimeError("invalid state for pin");

            int pb = PinBit(toWrite);
            ArduinoPort port = PinPort(toWrite);
            if (port == PORT_B) {
                DDRB |= (1 << pb); // set port b as output
                DRPORTB |= (state << pb);
            } else if (port == PORT_C) {
                DDRC |= (1 << pb); // set port c as output
                DRPORTC |= (state << pb);
            } else if (port == PORT_D) {
                DDRD |= (1 << pb); // set port d as output
                DRPORTD |= (state << pb);
            } else
                RuntimeError("invalid pin");
#endif
        }
        break;
    }
    case OP_PUSH:
        if (inst.data.value.type == TY_STR &&
            GetRegisterFromName((char*)inst.data.value.data.ptr) != REG_UNKNOWN) {
            // push from memory
            Push(machine, machine->memory[GetRegisterFromName((char*)inst.data.value.data.ptr)]);
            break;
        }
        Push(machine, inst.data.value);
        break;
    case OP_POP: {
        int val = Pop(machine);

        // pop to memory
        if (inst.data.registers.dest != REG_NONE)
            machine->memory[inst.data.registers.dest] = DATA_USING_I64(val);

        break;
    }
    case OP_SHL: {
        int val = Pop(machine);

        Push(machine, DATA_USING_I64(val << inst.data.value.data.i64));
        break;
    }
    case OP_ORB: {
        int b = Pop(machine);
        int a = Pop(machine);
        Push(machine, DATA_USING_I64(a | b));
        break;
    }
#ifndef USING_ARDUINO
    case OP_PRNT:
        PrintStack(machine);
        break;
    case OP_EXIT:
        printf("exiting with code %ld.\n", machine->memory[REG_RAX].data.i64);
        exit(machine->memory[REG_RAX].data.i64); // exit code saved in RAX register
    case OP_JLE: {
        JUMP_IF(<=, machine, inst.data.value.data.i64, &jump);
        break;
    }
    case OP_JL: {
        JUMP_IF(<, machine, inst.data.value.data.i64, &jump);
        break;
    }
    case OP_JGE: {
        JUMP_IF(>=, machine, inst.data.value.data.i64, &jump);
        break;
    }
    case OP_JG: {
        JUMP_IF(>, machine, inst.data.value.data.i64, &jump);
        break;
    }
    case OP_JE: {
        JUMP_IF(==, machine, inst.data.value.data.i64, &jump);
        break;
    }
    case OP_JNE: {
        JUMP_IF(!=, machine, inst.data.value.data.i64, &jump);
        break;
    }
    case OP_JMP:
        jump = TRUE;
        JumpTo(machine, inst.data.value.data.i64);
        break;
    case OP_NOP:
        break;
    case OP_SHR: {
        int val = Pop(machine);
        Push(machine, DATA_USING_I64(val >> inst.data.value.data.i64));
        break;
    }
    case OP_SWAP: {
        int first = Pop(machine);
        int second = Pop(machine);
        Push(machine, DATA_USING_I64(second));
        Push(machine, DATA_USING_I64(first));
        break;
    }
    case OP_MUL: {
        int a = Pop(machine);
        int b = Pop(machine);
        Push(machine, DATA_USING_I64(a * b));
        break;
    }
    case OP_DUP:
        Push(machine, machine->stack[machine->stackSize - 1]);
        break;
    case OP_ANDB: {
        int b = Pop(machine);
        int a = Pop(machine);
        Push(machine, DATA_USING_I64(a & b));
        break;
    }
    case OP_XORB: {
        int b = Pop(machine);
        int a = Pop(machine);
        Push(machine, DATA_USING_I64(a ^ b));
        break;
    }
    case OP_NOTB: {
        int a = Pop(machine);
        Push(machine, DATA_USING_I64(~a));
        break;
    }
    case OP_NEG: {
        int val = Pop(machine);
        val *= -1;
        Push(machine, DATA_USING_I64(val));
        break;
    }
    case OP_ADD: {
        int a = Pop(machine);
        int b = Pop(machine);
        Push(machine, DATA_USING_I64(a + b));
        break;
    }
    case OP_DIV: {
        if (machine->stack[machine->stackSize].data.i64 == 0) {
            fprintf(stderr, "Divide by zero error.\n");
            exit(1);
        }

        int b = Pop(machine);
        int a = Pop(machine); // you put the bigger number first in div

        Push(machine, DATA_USING_I64(a / b));
        break;
    }
    case OP_MOD: {
        int b = Pop(machine);
        int a = Pop(machine);
        Push(machine, DATA_USING_I64(a % b));
        break;
    }
    case OP_MOV:
        Move(machine, inst.data.value, inst.data.registers.dest);
        break;
    case OP_SUB: {
        int b = Pop(machine);
        int a = Pop(machine);
        Push(machine, DATA_USING_I64(a - b));
        break;
    }
    case OP_CLR:
        ClearStack(machine);
        break;
    case OP_SIZE:
        Push(machine, DATA_USING_I64(machine->stackSize));
        break;
#endif
    default:
        RuntimeError("\n\tIn 'RunInstructions()' : unknown instruction");
    }

    if (jump == FALSE) // was not a jump instruction
        machine->ip++;

    // run the next instruction in instruction pointer
    RunInstructions(machine);
}