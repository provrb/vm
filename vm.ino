/// This is the arduino sections of the project
///
/// The goal is to make my language run on an arduino, using
/// specialized instructions to control the hardware.


extern "C" {
    #include "api/lexer.h"
    #include <stdlib.h>
};

// constants
const uint8_t led = 11;

void interpret() {

    Serial.println("Instructions parsed");
    Instruction insts[4];

    insts[0].operation = OP_PUSH;
    insts[0].data.value = DATA_USING_I64(1);

    insts[1].operation = OP_PUSH;
    insts[1].data.value = DATA_USING_I64(11);

    insts[2].operation = OP_PUSH;
    insts[2].data.value = DATA_USING_I64(1);

    insts[3].operation = OP_WRITE;

    Serial.println(insts[0].data.value.data.i64);
    Serial.println(insts[1].data.value.data.i64);
    Serial.println(insts[2].data.value.data.i64);

    Machine* machine = (Machine*)malloc(sizeof(Machine));
    machine->stackSize = 0;
    machine->ip = 0;
    machine->program = insts;
    machine->rp = -1;
    machine->ep = -1;
    machine->programSize = sizeof(insts) / sizeof(Instruction);
    machine->labels[0] = Label{"start", 5, 0};
    machine->numLabels = 1;

    RunInstructions(machine);

    Serial.println("-------STACK--------");
    for (int i=0; i<machine->stackSize; i++) {
        Serial.println((char*)machine->stack[i].data.ptr);
    }
    Serial.println("-------STACKEND--------");
    Serial.println(machine->memory[REG_R10].data.i64);
    Serial.println(machine->memory[REG_R11].data.i64);
    Serial.println(machine->memory[REG_R12].data.i64);

    // PrintRegisterContents(machine);
}

void setup() {
    Serial.begin(9600);
    Serial.println("Live");

    interpret();
}

void loop() {}