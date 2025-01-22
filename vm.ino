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
String total = "";

void interpret() {
    Lexer lexer = ParseTokens(const_cast<char*>(total.c_str()));
    Instruction* insts = (Instruction*)malloc(lexer.numTokens * sizeof(Instruction));
    Serial.println(lexer.numTokens);
    for (unsigned int i = 0; i < lexer.numTokens; i++) {
        insts[i] = lexer.tokens[i].inst;
        // Serial.println(lexer.tokens[i].line);
        // PrintToken(&lexer.tokens[i]);
    }
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

void setup() {
    Serial.begin(9600);
    Serial.println("Live");
}

void loop() {
    if (Serial.available()) {
        String received = Serial.readStringUntil('\n'); // Read until newline
        if (received.equals("EOF")){
            Serial.println("Received all contents.");
            Serial.print(total);
            interpret();
            return;
        }
        
        received += '\n';
        total += received; // Append received data to total
    }
}