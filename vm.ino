/// This is the arduino sections of the project
///
/// The goal is to make my language run on an arduino, using
/// specialized instructions to control the hardware.


extern "C" {
    #include "api/lexer.h"
    #include <stdlib.h>
    #include <stdio.h>
};

int led = 11;

void interpret(String text) {
    Lexer lexer = ParseTokens("");
    Instruction* insts = (Instruction*)malloc(lexer.numTokens * sizeof(Instruction));
    for (unsigned int i = 0; i < lexer.numTokens; i++) {
        insts[i] = lexer.tokens[i].inst;
        PrintToken(&lexer.tokens[i]);
    }
    Machine* machine = (Machine*)malloc(sizeof(Machine));
    machine->stackSize = 0;
    machine->ip = 0;
    machine->program = insts;
    machine->rp = -1;
    machine->ep = -1;
    machine->programSize = lexer.numTokens;
    for (int i = 0; i < lexer.numLabels; i++) {
        // Serial.println("label: %s\n", lexer.labels[i].name);
        machine->labels[i] = lexer.labels[i];
    }
    machine->numLabels = lexer.numLabels;

    RunInstructions(machine);
    PrintRegisterContents(machine);
}

void setup() {
    Serial.begin(9600);
    Serial.println("Live");
}

String total = "";

void loop() {
    if (Serial.available() > 0) { // Check if data is available
        String received = Serial.readStringUntil('\n'); // Read until newline
        if (received.length() > 0) { // Ensure valid input
            Serial.println("Received:");
            Serial.println(received);
            total += received; // Append received data to total
            Serial.println("Total:");
            Serial.println(total);
        }
    }
}