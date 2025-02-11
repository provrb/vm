/// This is the arduino sections of the project
///
/// The goal is to make my language run on an arduino, using
/// specialized instructions to control the hardware.

/// Headers
extern "C" {
    #include "src/lexer.h" // Instruction, DATA_USING.., Machine
    #include <stdlib.h> // malloc
};

/// @brief Read the state of a pin using Instruction set 
/// @param led 
/// @return 
int low_level_read(int led) {
    Instruction insts[3];
    
    //  this is the pin to read
    insts[0].operation = OP_PUSH;
    insts[0].data.value = DATA_USING_I64(led);

    // push 1 to the stack, this is the file descriptor
    insts[1].operation = OP_PUSH;
    insts[1].data.value = DATA_USING_I64(FILE_INOPIN);

    // write to the pin
    insts[2].operation = OP_READ;

    Machine machine = {0};
    machine.labels[0] = Label{"start", 5, 0};
    machine.rp = -1;
    machine.numLabels = 1;
    machine.program = insts;
    machine.programSize = sizeof(insts) / sizeof(Instruction);

    RunInstructions(&machine);

    return Pop(&machine);
}

/// Call the high level 'OP_WRITE' instruction 
/// in the vm to compute and write to arduino pin
/// instead of doing it manually
/// @param led - pin number to write to
/// @param on - boolean, high or low state
/// @param machine - machine context
void high_level_write(int led, BOOL on, Machine* machine) {
    // Simulator instructions from the lexer
    Instruction insts[4];

    //  this is the pin to read
    insts[0].operation = OP_PUSH;
    insts[0].data.value = DATA_USING_I64(led);

    // push 1 to the stack, this is the file descriptor
    insts[1].operation = OP_PUSH;
    insts[1].data.value = DATA_USING_I64(FILE_INOPIN);

    // write to the pin
    insts[2].operation = OP_WRITE;

    machine->program = insts;
    machine->programSize = sizeof(insts) / sizeof(Instruction);

    RunInstructions(machine);
}

/// do the computations for digital write with
/// instructions and don't use OP_WRITE
/// use instructions like bit shift left, or
/// @param led - pin number to write to
/// @param on - boolean, high or low state
/// @param machine - machine context
void low_level_write(int led, BOOL on, Machine* machine) {
    unsigned short dd = 0x0;
    unsigned short port = 0x0;
    volatile unsigned char* ddptr = NULL;
    volatile unsigned char* pptr = NULL;

    // Choose correct DDR and PORT register
    if (led >= 0 && led <= 7) {
        ddptr = &DDRD;
        pptr = &PORTD;
    } else if (led >= 8 && led <= 13) {
        ddptr = &DDRB;
        pptr = &PORTB;
    } else if (led >= 14 && led <= 19) {
        ddptr = &DDRC;
        pptr = &PORTC;
    }

    dd = (unsigned char)&ddptr;
    port = (unsigned char)&pptr;

    Instruction insts[10];

    // Set DDRB |= (1 << pb) (configure pin as output)
    // push 36 to the stack (DDRB register value)
    insts[0].operation = OP_PUSH;
    insts[0].data.value = DATA_USING_I64(dd);

    // push 1 to the stack (bit shift operand)
    insts[1].operation = OP_PUSH;
    insts[1].data.value = DATA_USING_I64(1);

    // perform left shift 1 << 3 (bit position pb)
    insts[2].operation = OP_SHL;
    insts[2].data.value = DATA_USING_I64(PinBit(led));

    // OR DDRB with (1 << pb)
    insts[3].operation = OP_ORB;

    // pop result into DDRB
    insts[4].operation = OP_POP;
    insts[4].data.registers.dest = REG_R11;

    // Write state to pin (DRPORTB |= (state << pb))
    // push 37 to the stack (DRPORTB register value)
    insts[5].operation = OP_PUSH;
    insts[5].data.value = DATA_USING_I64(port);

    // push state (1 for HIGH) to the stack
    insts[6].operation = OP_PUSH;
    insts[6].data.value = DATA_USING_I64(on);

    // perform left shift state << pb
    insts[7].operation = OP_SHL;
    insts[7].data.value = DATA_USING_I64(PinBit(led));

    // OR DRPORTB with (state << pb)
    insts[8].operation = OP_ORB;
    
    // pop result into DRPORTB
    insts[9].operation = OP_POP;
    insts[9].data.registers.dest = REG_R10;

    machine->program = insts;
    machine->programSize = sizeof(insts) / sizeof(Instruction);

    RunInstructions(machine);

    *ddptr |= machine->memory[REG_R11].data.i64;
    *pptr |= machine->memory[REG_R10].data.i64;
}

void low_level_analog_write(int led, int value) {
    Instruction insts[3];

    insts[0].operation = OP_PUSH;
    insts[0].data.value = DATA_USING_I64(value);

    insts[1].operation = OP_PUSH;
    insts[1].data.value = DATA_USING_I64(led);

    insts[2].operation = OP_ANWRITE;

    Machine machine = {0};
    machine.labels[0] = Label{"start", 5, 0};
    machine.rp = -1;
    machine.numLabels = 1;
    machine.program = insts;
    machine.programSize = sizeof(insts) / sizeof(Instruction);

    RunInstructions(&machine);
}

void setup() {
    Serial.begin(9600);

    // low_level_write(11, TRUE, machine);
    // high_level_write(11, TRUE, machine);
}

void loop() {

    // Serial.println(low_level_read(11));
    // int buttonState = digitalRead(11);
    // Serial.println(buttonState);
    low_level_analog_write(9, 50);
    low_level_analog_write(10, 100);
    // low_level_analog_write(9, 255);
}