#include "inst.h"
#include "macros.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const RegisterMap registerMap[] = {
    {"rax", REG_RAX}, {"rbx", REG_RBX}, {"rcx", REG_RCX},     {"rdx", REG_RDX}, {"r8", REG_R8},
    {"r9", REG_R9},   {"r10", REG_R10}, {"r11", REG_R11},     {"r12", REG_R12}, {"r13", REG_R13},
    {"r14", REG_R14}, {"r15", REG_R15}, {"ep", REG_EP}, {"none", REG_UNKNOWN}
};

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

void Move(Machine* machine, int src, int dest) {
    printf("Moving from %d to %d\n", src, dest);

    char* regSrc = GetRegisterName(src);
    if (strcmp(regSrc, "unknown") == 0) {
        fprintf(stderr, "Invalid source register. Aborted.\n");
        exit(1);
    }

    char* regDest = GetRegisterName(dest);
    if (strcmp(regDest, "unknown") == 0) {
        fprintf(stderr, "Invalid destination register. Aborted.\n");
        exit(1);
    }

    machine->memory[dest] = machine->memory[src];
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
    printf("set ip: %d\n", dest);
    if (dest > machine->programSize || dest < 0) {
        return;
    }

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

void RuntimeError(Machine* machine, char* msg) {
    fprintf(stderr, "runtime error. %s\n", msg);
    exit(1);
}

int GetEntryPoint(Machine* machine) {
    for (int i = 0; i<machine->numLabels; i++) {
        printf("comparing %s to %s\n", LABEL_ENTRY_PNT, machine->labels[i].name);
        if (strcmp(LABEL_ENTRY_PNT, machine->labels[i].name) == 0)
            return machine->labels[i].index;
    }
    
    RuntimeError(machine, "no entry point"); // no entry point
}

void RunInstructions(Machine* machine) {
    if (machine->ip == 0 && machine->started == FALSE) {
        machine->ip = GetEntryPoint(machine);
        printf("entry point index: %d\n", machine->ip);
        machine->started = TRUE;
    }

    if (machine->ip >= machine->programSize) {
        return;
    }

    Instruction* instructions = (Instruction*)machine->program;
    if (!instructions) {
        fprintf(stderr, "Panic: Instructions invalid!\n");
        return;
    }

    Instruction inst = instructions[machine->ip];
    int jump = FALSE; // if inst.operation is a successful jump

    switch (inst.operation) {
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
    case OP_SHL: {
        int val = Pop(machine);

        Push(machine, DATA_USING_I64(val << inst.data.value.data.i64));
        break;
    }
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
    case OP_ORB: {
        int b = Pop(machine);
        int a = Pop(machine);
        Push(machine, DATA_USING_I64(a | b));
        break;
    }
    case OP_NEG: {
        int val = Pop(machine);
        val *= -1;
        Push(machine, DATA_USING_I64(val));
        break;
    }
    case OP_PUSH:
        if (inst.data.value.type == TY_STR &&
            GetRegisterFromName((char*)inst.data.value.data.ptr) != REG_UNKNOWN) {
            // push from memory
            Push(machine, machine->memory[GetRegisterFromName((char*)inst.data.value.data.ptr)]);
            break;
        }
        // printf("pushing %s\n", (char*)inst.data.value.data.ptr);
        Push(machine, inst.data.value);
        break;
    case OP_POP: {
        int val = Pop(machine);

        // pop to memory
        if (inst.data.registers.dest != REG_NONE)
            machine->memory[inst.data.registers.dest] = DATA_USING_I64(val);

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
        Move(machine, inst.data.registers.src, inst.data.registers.dest);
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
    case OP_PRNT:
        PrintStack(machine);
        break;
    case OP_EXIT:
        exit(machine->memory[REG_RAX].data.i64); // exit code saved in RAX register 
    default:
        fprintf(stderr, "error unknown opcode. exiting");
        exit(1);
    }

    if (jump == FALSE) // was not a jump instruction
        machine->ip++;

    // run the next instruction in instruction pointer
    RunInstructions(machine);
}