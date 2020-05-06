# MIPS  Assembler

## Usage

+ convert `.hex` to `.asm`

  ```bash
  MIPS -x $pathToHex$ [-o $pathToAsm$] #--translate
  ```

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

`NOTICE`:

1. Input files can have arbitrary name, they are not required to be in format of `*.asm` or whatever. But in this document, I will use specific name to represent files of specific format.

2. Check the format of your file content.

   For `.hex`, please align instructions code in 8 hexes line by line as follows:

   ```bash
   #content of *.hex
   08000008,
   00000020,
   00000020,
   ......
   #`,` can be ignored
   ```

   For `.coe`, please start with the specific header shown below and then layout code line by line in the same format with `.hex`

   ```bash
   memory_initialization_radix=16;
   memory_initialization_vector=
   08000008,
   .....
   08000008;
   ```

3. `a.out` will be the output if you do not specify the name of output by `-o/--out`.

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

  