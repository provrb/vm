#include "inst.h"
#include "macros.h"

#include <stdio.h>
#include <stdlib.h>

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
    if (machine->stackSize <= src || dest < 0 || src < 0 || machine->stackSize <= dest) {
        fprintf(stderr, "Trying to move value from %d to %d. Out-of-bounds. Aborted.\n", src, dest);
        exit(1);
    }
    machine->stack[dest] = machine->stack[src];
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

void RunInstructions(Machine* machine) {
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
        Push(machine, inst.data.value);
        break;
    case OP_POP:
        Pop(machine);
        break;
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
    default:
        fprintf(stderr, "error unknown opcode. exiting");
        exit(1);
    }

    if (jump == FALSE) // was not a jump instruction
        machine->ip++;

    // run the next instruction in instruction pointer
    RunInstructions(machine);
}