# Hack Assembler

A C++ implementation of the Hack Assembler for the Nand2Tetris course.  
It converts Hack assembly `.asm` files into binary `.hack` files.

## Usage

Compile the assembler:

```bash
g++ HackAssembler.cpp -o HackAssembler
```
Run it on a .asm file:
```
./HackAssembler Prog.asm
```
This will generate Prog.hack containing the machine code.

Features
- Supports A- and C-instructions.
- Handles symbols, labels, and variables.
- Outputs 16-bit Hack machine code.
