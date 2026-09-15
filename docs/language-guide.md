# Provrb VM language guide

This project is a tiny virtual machine and bytecode interpreter inspired by x86-style assembly. It reads a `.pvb` program, turns each line into tokens, and executes it with a custom stack-based machine.

The design is intentionally simple:

- Registers behave like a small CPU
- The stack is where temporary values live
- Labels let code branch and call other sections
- Syscalls expose a limited OS-like interface
- The runtime executes instructions one-by-one like a mini interpreter

This file is meant to help you remember how the language works, explain it to someone new, and explain the project to a recruiter without overwhelming them.

## 1. The big idea

Imagine a computer with:

- A few named boxes called registers (`rax`, `rbx`, `rdi`, etc.)
- A stack, like a pile of plates, where values are pushed and popped
- A program counter that knows which instruction is being executed next
- A few comparison flags to tell whether values are equal, greater, less, etc.

The VM executes instructions in order, unless a jump or call changes the flow.

The entry point is a label like:

```asm
_start:
    mov $5, rax
    push 10
    print
```

The underscore is part of the syntax, but the VM stores the label as `start` internally. So `_start:` is the special starting location for the program.

## 2. Basic syntax

The language is intentionally close to assembly. Each instruction is usually one keyword followed by operands.

Examples:

```asm
mov $5, rax
push 42
pop rbx
add rax, rbx
jmp _loop
cmp $10, rax
je _done
```

Rules:

- Comments start with `;`
- Labels end with `:`
- `$` means immediate literal value, like `mov $5, rax`
- Register names are lowercase (`rax`, `rbx`, `rcx`, `rdi`, `rsi`, `rdx`, `r8`, ...)
- String literals use double quotes
- Jump labels are referenced like `jmp _loop` or `call _helper`

## 3. Memory model

The VM keeps a simulated memory space and stack.

### Registers

The VM has general-purpose registers:

- `rax`  - often used as a return value or syscall result
- `rbx`
- `rcx`
- `rdi` - first syscall argument
- `rsi` - second syscall argument
- `rdx` - third syscall argument
- `r8`  - fifth syscall argument
- `r9`  - sixth syscall argument
- `r10` - fourth syscall argument
- `r11`, `r12`, `r13`, `r14`, `r15`
- `ep` and `cp` are also reserved for execution flow

There is no full CPU, but the register naming is intentionally inspired by x86-64 calling conventions.

### Stack

The stack is an array of `Data` values and behaves like a last-in-first-out pile.

Examples:

```asm
push 10
push 20
pop rax   ; rax = 20
pop rbx   ; rbx = 10
```

`push` adds to the top of the stack, and `pop` removes the top value.

## 4. Data types

The runtime supports several data shapes:

- `i64` - signed 64-bit integer
- `u64` - unsigned 64-bit integer
- `f64` - floating point
- `str` - string literal
- `reg` references are represented as integer register IDs internally

Examples:

```asm
mov $42, rax
mov $3.14, rbx
push "hello"
```

## 5. Instructions reference

The instruction set is the core of the language. The actual opcodes are defined in `src/inst.h`.

### Data movement and stack

- `mov src, dest`
  - Copy a value or register into another register.
  - Example: `mov $5, rax`
  - Example: `mov rbx, rax`

- `push value`
  - Push a literal, register value, or string onto the stack.
  - Example: `push 10`
  - Example: `push rax`

- `pop [register]`
  - Remove the top of the stack.
  - If a register is supplied, write the value into it.
  - Example: `pop rax`

- `swap`
  - Swap the top two stack values.

- `dup`
  - Duplicate the top stack value.

- `clear`
  - Empty the stack.

- `size`
  - Push the current stack size onto the stack.

### Arithmetic

- `add a, dest`
- `sub a, dest`
- `mul a, dest`
- `div a, dest`
- `mod a, dest`
- `neg`

These operations work on integer/float values and overwrite the destination register or the stack value depending on context.

Examples:

```asm
mov $10, rax
mov $5, rbx
add $5, rax     ; rax = 15
sub rbx, rax    ; rax = 10
mul $2, rax     ; rax = 20
```

### Bitwise operations

- `AND`
- `OR`
- `NOT`
- `XOR`
- `shl amount`
- `shr amount`

These are stack-based and a bit closer to low-level operations.

Example:

```asm
push 1
shl 3
pop r10
```

This shifts `1` left by 3, resulting in `8`.

### Comparison and jumps

- `cmp value, reg`
  - Subtracts the compared value from the register and sets flags.
  - This is the same idea as CPU compares.

Flag behavior is tracked with:

- `ZF` - zero flag
- `SF` - sign flag
- `OF` - overflow flag

Then the jump instructions use those flags:

- `jmp label`
- `je label` - jump if equal
- `jne label` - jump if not equal
- `jg label` - jump if greater
- `jge label` - jump if greater or equal
- `jl label` - jump if less
- `jle label` - jump if less or equal

Example:

```asm
_start:
    mov $10, rax
    cmp $10, rax
    je _equal
    mov $1, rax
    exit

_equal:
    mov $0, rax
    exit
```

### Calls and returns

- `call label`
  - Saves the current instruction pointer and jumps to a function-like label.

- `ret`
  - Returns to the saved return pointer.

Example:

```asm
_start:
    call _helper
    mov $42, rax
    exit

_helper:
    mov $7, rax
    ret
```

### Input/output

- `read`
  - Reads from stdin and pushes the string onto the stack.

- `write`
  - Writes a string or output to a file descriptor.

- `print`
  - Prints the current stack contents.

Examples:

```asm
read
pop rax
```

```asm
push "hello, world"
push 1
write
```

The VM also has Arduino-specific I/O support when compiled with `USING_ARDUINO`.

### Process control

- `exit`
  - Halts the VM. It reads the exit code from `rax`.

- `nop`
  - No operation.

## 6. Syscalls

The language includes a custom syscall interface modeled after Linux-style conventions. The actual syscall numbers are in `src/inst.h`.

The VM uses registers like this:

- `rax` = syscall number
- `rdi` = arg1
- `rsi` = arg2
- `rdx` = arg3
- `r10` = arg4
- `r8` = arg5
- `r9` = arg6

Then you call:

```asm
syscall
```

### Supported syscall numbers

| Number | Name | What it does |
| --- | --- | --- |
| 1 | `SYS_EXEC` | Declared but not implemented in the switch logic |
| 2 | `SYS_ALLOC` | Allocates memory using `mmap` / `VirtualAlloc` |
| 3 | `SYS_FREE` | Fuses memory deallocation logic |
| 4 | `SYS_REALLOC` | Declared but not implemented |
| 5 | `SYS_PROTECT` | Memory protection / `mprotect` |
| 6 | `SYS_ENV` | Declared but not implemented |
| 7 | `SYS_SLEEP` | Sleeps for a given number of seconds |
| 8 | `SYS_CYCLES` | Returns instruction cycles count |

Important note: the runtime only fully implements a subset of them. The rest exist as placeholders in the enum.

### Syscall example: allocate memory

```asm
_start:
    mov $2, rax      ; SYS_ALLOC
    mov $0, rdi      ; NULL
    mov $100, rsi    ; length
    mov $4096, rdx   ; protection flags
    mov $64, r10     ; mapping flags
    syscall
    mov rax, rdi
    exit
```

This stores the syscall number in `rax` and then passes arguments in the corresponding registers. After `syscall`, the result is usually found in `rax`.

### Syscall example: sleep

```asm
_start:
    mov $7, rax      ; SYS_SLEEP
    mov $2, rdi      ; sleep for 2 seconds
    syscall
    exit
```

### Syscall example: cycles counter

```asm
_start:
    mov $8, rax      ; SYS_CYCLES
    syscall
    mov rax, rbx
    exit
```

This returns the number of instructions executed so far.

## 7. Example programs

### Hello world-like example

```asm
_start:
    push "hello"
    push 1
    write
    exit
```

This is not a full OS-level print, but it demonstrates the stack plus output flow.

### Arithmetic flow

```asm
_start:
    mov $5, rax
    mov $7, rbx
    add rbx, rax
    push rax
    print
    exit
```

### Loop example with labels

```asm
_start:
    mov $0, rax

_loop:
    cmp $5, rax
    je _done
    add $1, rax
    jmp _loop

_done:
    exit
```

This starts at zero, increments until it reaches five, then exits.

## 8. How the interpreter works internally

The interpreter is split into a few major pieces:

1. Lexer
   - Reads source text from a `.pvb` file
   - Finds labels, commands, and operands
   - Converts them into tokens

2. Instruction encoding
   - Each parsed line becomes an `Instruction`
   - Opcodes and operands are stored in a compact C struct

3. Runtime loop
   - The machine runs `RunInstructions()` repeatedly
   - It reads the current instruction from `machine->ip`
   - It executes the opcode and increments or updates the instruction pointer

4. Control flow
   - `call` stores return locations
   - `jmp` and conditional jumps change where execution continues
   - `cmp` updates flags for decision-making

5. Memory model
   - Registers are stored in a memory array
   - The stack is separate from the register file

This is a classic compiler/runtime pattern:

- parse source text
- lower it to bytecode/instructions
- execute the instruction stream in a VM

## 9. Teaching it to someone with no programming background

If someone knows almost nothing about code, use this analogy:

Think of the VM as a tiny robot with a few labeled boxes and a stack of cards.

- `rax`, `rbx`, `rcx` are boxes
- `push` places a card on the top of the stack
- `pop` takes the top card away
- `mov` copies a number into a box
- `add` or `sub` adds or subtracts values
- `cmp` compares two values and asks, “Are they the same?”
- `jmp` tells the robot, “Go to this instruction next”
- `call` says, “Remember where you were, then go to this helper task”
- `ret` says, “Go back to where you left off”

A program is just a list of instructions the robot follows.

## 10. Recruiter-friendly explanation

If you want a short explanation for a recruiter or hiring manager, use something like this:

> I built a custom virtual machine and assembly-like language in C. It includes a lexer, parser, instruction set, stack-based execution model, register file, labels, jumps, memory operations, and syscall interface. The project demonstrates systems-level thinking: tokenization, runtime execution, control flow, low-level data modeling, and interpreted execution. It’s similar to how a real CPU or scripting runtime works, but on a smaller, custom scale.

This is good because it highlights:

- parsing and interpretation
- systems programming
- low-level architecture
- memory and control flow
- deliberate design thinking

It sounds technical without getting lost in syntax details.

## 11. Quick summary

This project is basically a tiny custom CPU in software.

It has:

- registers
- labels
- jumps
- stack operations
- arithmetic and bitwise operations
- syscall API
- an interpreter loop

That means it touches a lot of real computer science ideas in one small codebase: parsing, control flow, execution semantics, memory, and systems programming.

## 12. Where to look in the repo

- `src/lexer.c` - parses `.pvb` source into tokens
- `src/inst.h` - instruction definitions, syscall enum, register map
- `src/inst.c` - runtime execution engine and syscall implementations
- `examples/*.pvb` - example programs
- `README.md` - project overview and entry point

## 13. Practical tip

When learning the language, think in this order:

1. Put values in registers with `mov`
2. Move values to and from the stack with `push` and `pop`
3. Compare with `cmp`
4. Use jump instructions to control flow
5. Call helper labels with `call`
6. Use `syscall` for OS-like operations

That pattern will make the rest of the instructions much easier to understand.
