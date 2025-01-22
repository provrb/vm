#include <Arduino.h>
#line 1 "/home/ethan/Documents/provrb/vm/vm.ino"
/// This is the arduino sections of the project
///
/// The goal is to make my language run on an arduino, using
/// specialized instructions to control the hardware.

#define USING_ARDUINO

extern "C" {
    #include "api/lexer.h"
    #include <stdlib.h>
    #include <stdio.h>
};

int led = 11;

#line 16 "/home/ethan/Documents/provrb/vm/vm.ino"
void setup();
#line 50 "/home/ethan/Documents/provrb/vm/vm.ino"
void loop();
#line 16 "/home/ethan/Documents/provrb/vm/vm.ino"
void setup() {
    Serial.begin(9600);
    Serial.println("Live");
    pinMode(led, OUTPUT);

    // Lexer lexer = ParseTokens("");

    // Instruction* insts = (Instruction*)malloc(lexer.numTokens * sizeof(Instruction));
    // for (unsigned int i = 0; i < lexer.numTokens; i++) {
    //     insts[i] = lexer.tokens[i].inst;
    //     PrintToken(&lexer.tokens[i]);
    // }

    // Machine* machine = (Machine*)malloc(sizeof(Machine));
    // machine->stackSize = 0;
    // machine->ip = 0;
    // machine->program = insts;
    // machine->rp = -1;
    // machine->ep = -1;
    // machine->programSize = lexer.numTokens;
    // for (int i = 0; i < lexer.numLabels; i++) {
    //     // Serial.println("label: %s\n", lexer.labels[i].name);
    //     machine->labels[i] = lexer.labels[i];
    // }

    // machine->numLabels = lexer.numLabels;
    

    // RunInstructions(machine);
    // PrintRegisterContents(machine);
}

String total = "";

void loop() {
    if (Serial.available()) {
        String received = Serial.readStringUntil('\n');
        Serial.print("Received: ");
        total+=received;
    }
}
