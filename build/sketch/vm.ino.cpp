#include <Arduino.h>
#line 1 "C:\\Users\\ethan\\Desktop\\vm\\vm.ino"
/// This is the arduino sections of the project
///
/// The goal is to make my language run on an arduino, using
/// specialized instructions to control the hardware.

/// Headers
extern "C" {
    #include "src/lexer.h" // Instruction, DATA_USING.., Machine
    #include <stdlib.h> // malloc
};

/// constants
const unsigned short led = 11; // led pin

/// @brief Interpret a mock program
#line 16 "C:\\Users\\ethan\\Desktop\\vm\\vm.ino"
void interpret();
#line 52 "C:\\Users\\ethan\\Desktop\\vm\\vm.ino"
void setup();
#line 59 "C:\\Users\\ethan\\Desktop\\vm\\vm.ino"
void loop();
#line 16 "C:\\Users\\ethan\\Desktop\\vm\\vm.ino"
void interpret() {
    Serial.println("Interpreting...");
    
    // Simulator instructions from the lexer
    Instruction insts[4];

    // push 1 to the stack, this is the file descriptor
    insts[0].operation = OP_PUSH;
    insts[0].data.value = DATA_USING_I64(1);

    // push 11 to the stack, this is the pin to write to
    insts[1].operation = OP_PUSH;
    insts[1].data.value = DATA_USING_I64(11);

    // push 1 (true) to the stack, this is the state, led on
    insts[2].operation = OP_PUSH;
    insts[2].data.value = DATA_USING_I64(1);

    // write to the pin
    insts[3].operation = OP_WRITE;

    // Create a machine
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
    Serial.println("Successfully ran instructions.");
}

void setup() {
    Serial.begin(9600);
    Serial.println("Live");

    interpret();
}

void loop() {}
