#include "inst.h"
#include "macros.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__)
#include <sys/mman.h>
#include <unistd.h>
#endif

static const RegisterMap registerMap[] = {
    {"rax", REG_RAX}, {"rdi", REG_RDI},     {"rsi", REG_RSI}, {"rbx", REG_RBX}, {"rcx", REG_RCX},
    {"rdx", REG_RDX}, {"r8", REG_R8},       {"r9", REG_R9},   {"r10", REG_R10}, {"r11", REG_R11},
    {"r12", REG_R12}, {"r13", REG_R13},     {"r14", REG_R14}, {"r15", REG_R15}, {"ep", REG_EP},
    {"cp", REG_CP},   {"none", REG_UNKNOWN}};

static const Syscall syscalls[] = {
    SYS_EXEC, SYS_ALLOC, SYS_FREE,   SYS_REALLOC, SYS_PROTECT,
    SYS_ENV,  SYS_SLEEP, SYS_CYCLES, SYS_UNKNOWN,
};

BOOL ValidSyscall(unsigned int ssn) {
    for (int i = 0; syscalls[i] != SYS_UNKNOWN; i++)
        if (syscalls[i] == ssn)
            return TRUE;

    return FALSE;
}

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

Data DATA_USING_F64(double val) {
    Data d = {.data.f64 = val, .type = TY_F64};
    return d;
}

Data DATA_USING_PTR(void* ptr) {
    Data d = {.data.ptr = ptr, .type = TY_ANY};
    return d;
}

Data DATA_USING_I64(long val) {
    printf("val %d\n", val);
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

// https://stackoverflow.com/a/1997606
BOOL IsFloat(const char* s) {
    char* ep = NULL;
    long i = strtol(s, &ep, 10);

    if (!*ep)
        return FALSE;

    if (*ep == 'e' || *ep == 'E' || *ep == '.')
        return TRUE;

    return FALSE;
}

void Move(Machine* machine, Operand data, int dest) {
    const char* regDest = GetRegisterName(dest);
    if (strcmp(regDest, "unknown") == 0) {
        fprintf(stderr, "Invalid destination register. Aborted.\n");
        exit(1);
    }

    if (strcmp(GetRegisterName(data.data.i64), "unknown") == 0) {
        RemoveChar((char*)data.data.ptr, LXR_CONSTANT_PREFIX);
        if (IsFloat((char*)data.data.ptr) == TRUE) {
            char* endptr;
            double f = strtod((char*)data.data.ptr, &endptr);
            machine->memory[dest] = DATA_USING_F64(f);
        } else
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

Data PopData(Machine* machine) {
    if (machine->stackSize <= 0) {
        fprintf(stderr, "Stack underflow when trying to pop from stack. Aborted.\n");
        exit(1);
    }

    return machine->stack[--machine->stackSize];
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
        else if (x.type == TY_F64) {
            printf("%f\n", machine->stack[i].data.f64);
        }
    }
    printf("--- Stack End   ---\n");
}

void JumpTo(Machine* machine, int dest) {
    if (dest > machine->programSize || dest < 0)
        RuntimeError("Jumping out of bounds. Aborted.");

    machine->ip = dest;
}

void Call(Machine* machine, int dest) {
    if (dest > machine->programSize || dest < 0)
        RuntimeError("Calling out of bounds. Aborted.");

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
    for (int i = REG_RAX; i < REG_R15 + 1; i++) {
        Data data = machine->memory[i];
        printf("%-4s: ", GetRegisterName(i));

        switch (data.type) {
        case TY_F64:
            printf("%f (f64)", data.data.f64);
            break;
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
    return 0;
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
    machine->cycles++;

    if (machine->ip == 0 && machine->started == FALSE) {
        machine->ip = GetEntryPoint(machine);
        machine->started = TRUE;
    }

    if (machine->ip >= machine->programSize)
        return;

    int jump = FALSE; // if inst.operation is a successful jump
    Instruction inst = ((Instruction*)machine->program)[machine->ip];

    switch (inst.operation) {
    case OP_RET:
        machine->ip = machine->rp;
        break;
    case OP_CALL:
        jump = TRUE;
        Call(machine, inst.data.value.data.i64);
        break;
    case OP_READ: {
        unsigned int fd = Pop(machine);
        if (fd == FILE_INOPIN) {
#ifdef USING_ARDUINO
            int pin = Pop(machine);
            int pb = PinBit(pin);
            ArduinoPort port = PinPort(pin);
            if (port == PORT_B) {
                DDRB &= ~(1 << pb); // set port b as output
                Push(machine, DATA_USING_I64((INPB & (1 << pb)) >> pb));
            } else if (port == PORT_C) {
                DDRC &= ~(1 << pb); // set port c as output
                Push(machine, DATA_USING_I64((INPC & (1 << pb)) >> pb));
            } else if (port == PORT_D) {
                DDRD &= ~(1 << pb); // set port d as output
                Push(machine, DATA_USING_I64((INPD & (1 << pb)) >> pb));
            } else
                RuntimeError("invalid pin");
#endif
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
#ifdef USING_ARDUINO
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
#ifdef USING_ARDUINO
    case OP_ANWRITE: {
        unsigned int pin = Pop(machine);
        unsigned int value = Pop(machine);
        unsigned int pb = PinBit(pin);
        ArduinoPort port = PinPort(pin);

        // First, set the Data direction register for the pin to output
        if (port == PORT_B)
            DDRB |= (1 << pb);
        else if (port == PORT_C)
            DDRC |= (1 << pb);
        else if (port == PORT_D)
            DDRD |= (1 << pb);
        else
            RuntimeError("invalid pin port");

        // Set the timer/counter control register to fast pwm and non inverting mode
        // Set part b of the timer/counter prescaler to 8
        // finally, set the output compare register to the value to set the pin
        if (pin == 3) {
            TCCR2A |= (1 << COM2B1) | (1 << WGM20) | (1 << WGM21);
            TCCR2B |= (1 << CS21);
            OCR2B = value;
        } else if (pin == 5) {
            TCCR0A |= (1 << COM0B1) | (1 << WGM00) | (1 << WGM01);
            TCCR0B |= (1 << CS01);
            OCR0B = value;
        } else if (pin == 6) {
            TCCR0A |= (1 << COM0A1) | (1 << WGM00) | (1 << WGM01);
            TCCR0B |= (1 << CS01);
            OCR0A = value;
        } else if (pin == 9) {
            TCCR1A |= (1 << COM1A1) | (1 << WGM10) | (1 << WGM11);
            TCCR1B |= (1 << WGM12) | (1 << CS11);
            OCR1A = value;
        } else if (pin == 10) {
            TCCR1A |= (1 << COM1B1) | (1 << WGM10) | (1 << WGM11);
            TCCR1B |= (1 << WGM12) | (1 << CS11);
            OCR1B = value;
        } else if (pin == 11) {
            TCCR2A |= (1 << COM2A1) | (1 << WGM20) | (1 << WGM21);
            TCCR2B |= (1 << CS21);
            OCR2A = value;
        } else
            RuntimeError("invalid pin");

        break;
    }
#endif
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
        Data val = PopData(machine);

        // pop to memory
        if (inst.data.registers.dest == REG_NONE)
            break;

        machine->memory[inst.data.registers.dest] =
            (val.type == TY_F64) ? DATA_USING_F64(val.data.f64) : DATA_USING_I64(val.data.i64);

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
    case OP_PRNT:
        PrintStack(machine);
        break;
    case OP_EXIT:
        printf("exiting with code %ld.\n", machine->memory[REG_RAX].data.i64);
        exit(machine->memory[REG_RAX].data.i64); // exit code saved in RAX register
    case OP_JLE:
        if (machine->EFLAGS & FLAG_ZF ||
            (machine->EFLAGS & FLAG_SF) != (machine->EFLAGS & FLAG_OF)) {
            jump = TRUE;
            JumpTo(machine, inst.data.value.data.i64);
        }
        break;
    case OP_JL:
        if ((machine->EFLAGS & FLAG_SF) != (machine->EFLAGS & FLAG_OF)) {
            jump = TRUE;
            JumpTo(machine, inst.data.value.data.i64);
        }
        break;
    case OP_JGE:
        if (machine->EFLAGS & FLAG_ZF ||
            (machine->EFLAGS & FLAG_SF) == (machine->EFLAGS & FLAG_OF)) {
            jump = TRUE;
            JumpTo(machine, inst.data.value.data.i64);
        }
        break;
    case OP_JG:
        if (!(machine->EFLAGS & FLAG_ZF) &&
            (machine->EFLAGS & FLAG_SF) == (machine->EFLAGS & FLAG_OF)) {
            jump = TRUE;
            JumpTo(machine, inst.data.value.data.i64);
        }
        break;
    case OP_JE:
        if (machine->EFLAGS & FLAG_ZF) {
            jump = TRUE;
            JumpTo(machine, inst.data.value.data.i64);
        }
        break;
    case OP_JNE:
        if (!(machine->EFLAGS & FLAG_ZF)) {
            jump = TRUE;
            JumpTo(machine, inst.data.value.data.i64);
        }
        break;
    case OP_JMP:
        jump = TRUE;
        JumpTo(machine, inst.data.value.data.i64);
        break;
    case OP_NOP:
        break;
    case OP_SHR: {
        int val = Pop(machine);
        Push(machine, DATA_USING_I64(val >> inst.data.value.data.i64));
    } break;
    case OP_SWAP: {
        int first = Pop(machine);
        int second = Pop(machine);
        Push(machine, DATA_USING_I64(second));
        Push(machine, DATA_USING_I64(first));
    } break;
    case OP_SYSCALL: {
        // rax holds the ssn
        Data ssn = machine->memory[REG_RAX];
        if (ssn.type != TY_I64) {
            Move(machine, DATA_USING_I64(-1), REG_RAX);
            break;
        }

        // argument 1 for syscall
        Data arg1 = machine->memory[REG_RDI];
        Data arg2 = machine->memory[REG_RSI];
        Data arg3 = machine->memory[REG_RDX];
        Data arg4 = machine->memory[REG_R10];
        Data arg5 = machine->memory[REG_R8];
        Data arg6 = machine->memory[REG_R9];

        switch (ssn.data.i64) {
        case SYS_ALLOC: {
            void* baseAddress = NULL;
#ifdef _WIN32
            baseAddress =
                (arg1.data.i64 == 0)
                    ? VirtualAlloc(NULL, arg2.data.i64, arg3.data.i64, arg4.data.i64)
                    : VirtualAlloc(arg1.data.ptr, arg2.data.i64, arg3.data.i64, arg4.data.i64);
#elif defined(__linux__)
            baseAddress = (arg1.data.i64 == 0) ? mmap(NULL, arg2.data.i64, arg3.data.i64,
                                                      arg4.data.i64, arg5.data.i64, arg6.data.i64)
                                               : mmap(arg1.data.ptr, arg2.data.i64, arg3.data.i64,
                                                      arg4.data.i64, arg5.data.i64, arg6.data.i64);
#endif
            if (baseAddress == NULL) {
                printf("Error allocating memory\n");
                Move(machine, DATA_USING_I64(-1), REG_RAX);
                break;
            }

            // put result in rax register
            machine->memory[REG_RAX].data.i64 = baseAddress;
            machine->memory[REG_RAX].type = TY_I64;
        } break;
        case SYS_CYCLES:
            Move(machine, DATA_USING_I64(machine->cycles), REG_RAX);
            break;
        case SYS_FREE: {
            BOOL success = FALSE;
#ifdef _WIN32
            success = VirtualFree(arg1.data.i64, arg2.data.i64, arg3.data.i64);
#elif defined(__linux__)
            success = munmap(arg1.data.i64, arg2.data.i64);
            if (success = -1)      // on linux munmap returns -1 for failure
                success = FALSE;   // set to false to align with standards
            else if (success == 0) // returns 0 on success, set to TRUE
                success = TRUE;
#endif
            Move(machine, DATA_USING_I64(success), REG_RAX);
        } break;
        case SYS_SLEEP:
#ifdef _WIN32
            Sleep(arg1.data.i64);
#elif defined(__linux__)
            sleep(arg1.data.i64);
#endif
            break;
        case SYS_PROTECT: {
#ifdef _WIN32
            PDWORD oldProtect;
            BOOL success = VirtualProtect(arg1.data.i64, arg2.data.i64, arg3.data.i64, &oldProtect);
            Move(machine, DATA_USING_I64(success), REG_RAX);
            if (success == TRUE)
                Move(machine, DATA_USING_I64(oldProtect), REG_R10);
#elif defined(__linux__)
            BOOL success = mprotect(arg1.data.i64, arg2.data.i64, arg3.data.i64);
            if (success == 0)
                success = TRUE;
            else if (success == -1)
                success = FALSE;
            Move(machine, DATA_USING_I64(success), REG_RAX);
#endif
        } break;
        default: // -1 return value
            Move(machine, DATA_USING_I64(-1), REG_RAX);
            break;
        }
    } break;
    case OP_MUL: {
        ARITHMETIC(*, inst, machine, '*')
        break;
    }
    case OP_DUP:
        Push(machine, machine->stack[machine->stackSize - 1]);
        break;
    case OP_ANDB: {
        int b = Pop(machine);
        int a = Pop(machine);
        Push(machine, DATA_USING_I64(a & b));
    } break;
    case OP_XORB: {
        int b = Pop(machine);
        int a = Pop(machine);
        Push(machine, DATA_USING_I64(a ^ b));
    } break;
    case OP_NOTB: {
        int a = Pop(machine);
        Push(machine, DATA_USING_I64(~a));
    } break;
    case OP_NEG: {
        int val = Pop(machine);
        val *= -1;
        Push(machine, DATA_USING_I64(val));
    } break;
    case OP_CMP: {
        machine->EFLAGS = 0;

        long a = 0;
        if (strcmp(GetRegisterName(inst.data.value.data.i64), "unknown") == 0) {
            RemoveChar((char*)inst.data.value.data.ptr, LXR_CONSTANT_PREFIX);
            a = atol((char*)inst.data.value.data.ptr);
        } else
            a = machine->memory[inst.data.value.data.i64].data.i64;

        long b = machine->memory[inst.data.registers.dest].data.i64;
        long result = b - a;

        // zero flag
        if (result == 0)
            machine->EFLAGS |= FLAG_ZF;
        else
            machine->EFLAGS &= ~FLAG_ZF;

        // sign flag
        if (result < 0)
            machine->EFLAGS |= FLAG_SF;
        else
            machine->EFLAGS &= ~FLAG_SF;

        // overflow flag
        if ((b ^ a) & (b ^ result) < 0)
            machine->EFLAGS |= FLAG_OF;
        else
            machine->EFLAGS &= ~FLAG_OF;

    } break;
    case OP_ADD: {
        ARITHMETIC(+, inst, machine, '+')
    } break;
    case OP_DIV: {
        ARITHMETIC(/, inst, machine, '/')
    } break;
    case OP_MOD: {
        ARITHMETIC(/, inst, machine, '%')
    } break;
    case OP_MOV:
        Move(machine, inst.data.value, inst.data.registers.dest);
        break;
    case OP_SUB: {
        ARITHMETIC(-, inst, machine, '-')
    } break;
    case OP_CLR:
        ClearStack(machine);
        break;
    case OP_SIZE:
        Push(machine, DATA_USING_I64(machine->stackSize));
        break;
    default:
        RuntimeError("\n\tIn 'RunInstructions()' : unknown instruction");
    }

    if (jump == FALSE) // was not a jump instruction
        machine->ip++;

    // run the next instruction in instruction pointer
    RunInstructions(machine);
}