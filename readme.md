# MIPS  Assembler

## Usage

+ convert `.asm` to `.bin`

  ```bash
  MIPS -a $pathToAsm$ [-o $pathToBin$] #--assemble
  ```

+ convert `.bin` to `.coe`

  ```bash
  MIPS -c $pathToBin$ [-o $pathToCoe$] #--coe
  ```

+ convert `.coe` to `.asm`

  ```bash
  MIPS -d $pathToCoe$ [-o $pathToAsm$] #--disassemble
  ```

+ check version and help

  ```bash
  MIPS -v ##--version
  MIPS -h ##--help
  ```

## support instructions

**38** instructions in total:

+ `[symbol] reg, reg, reg`

  ```c++
  "add",	"addu",	"sub", "subu", 
  "and",	"or",	"xor", "nor", 
  "sllv",	"srlv",	"srav","slt",
  "sltu"
  ```

+ `[symbol] reg, reg, shamt(5 bits wide)`

  ```c++
  "sll",	"srl",	"sra"
  ```

+ `[symbol] reg, reg, immediate(16 bit wide)`

  ```c++
  "addi", "ori",	"addiu",
  "andi",	"xori", "slti",	"sltiu"
  ```

+ `[symbol] reg, immediate(16 bit wide)`

  ```c++
  "lui"
  ```

+ `[symbol] reg, offset([reg]), $offset$ of 32 bit wide `

  ```c++
  "lw",	"lb",	"lbu",	"lh",
  "lhu",	"sw",	"sh",	"sb"
  ```

+ `[symbol] reg`

  ```c++
  "jr"
  ```

+ `[symbol] reg, reg, label/immediate(16 bit wide)`

  ```c++
  "beq", "bne"
  ```

+ `[symbol] label/address(26 bit wide)`

  ```c++
  "j", "jal"
  ```

  