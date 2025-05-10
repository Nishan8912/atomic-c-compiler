# Atomic-C-Compiler

A lightweight, modular compiler for a C-like language, built from scratch using `lex` and `yacc`, targeting **x86-64 assembly**, with full support for **Valgrind-based memory safety validation**. This project demonstrates a complete compiler pipeline including lexical analysis, parsing, semantic analysis, and assembly code generation.

> Designed for educational and demonstration purposes  
> Suitable for showcasing compiler construction skills in systems programming and language processing

---

## Key Features

- **Lexical Analysis** using `lex` (Flex): Recognizes identifiers, keywords, literals, and symbols
- **Parsing** with `yacc` (Bison-compatible): Builds an Abstract Syntax Tree (AST) from a context-free grammar
- **Symbol Table** with scope, kind, and offset tracking for globals, locals, arrays, and parameters
- **Code Generation** to x86-64 assembly (AT&T syntax), compatible with Linux toolchains (GCC)
- **Control Flow Support**: 
  - `if-then-else`, `while`, `do-while` loops
  - `returnvalue` handling
- **Functionality**:
  - Global and local variable declarations
  - Global array support with indexed access
  - Function calls with argument passing and return values
  - Full operator support: `+`, `-`, `*`, `/`, `%`, `==`, `!=`, `<`, `>`, `<=`, `>=`, `!`, `&`, `|`

---

## Extra Enhancements

- Extended operator set (`>=`, `<=`, `!`, bitwise ops)
- Support for **parenthesized expressions**
- `do-while` loop implementation
- Improved error handling and code generation patterns

---

## Project Architecture
tiny-c-compiler/
├── astree.c # AST creation & recursive codegen
├── astree.h
├── symtable.c # Symbol table management
├── symtable.h
├── scanner.l # Lexer definitions
├── parser.y # Grammar rules and action blocks
├── Makefile # Build automation
├── test.j # Sample input program
├── test.s # Generated x86-64 assembly
└── test-programs/ # Additional test cases

> The `test-programs/` folder includes a variety of test cases for control structures used to validate compiler correctness and completeness.

---

## How to Build & Run

### Prerequisites:
- GCC and `make`
- `flex` and `yacc`
- valgrind (for memory leak detection)

### Compile Compiler:
```bash
make # Builds the compiler executable
make assembly    # Runs compiler on test.j and produces test.s
make test        # Compiles test.s into 'test' binary and runs it
make memcheck    # Runs Valgrind on compiler using test.j as input


### Highlights
- Built entirely from scratch using low-level tools

- Clear separation between compiler stages

- Real backend generation to assembly

- Well-documented and modular codebase

- Multiple test cases included for validation

### Author
- Nishan Rai
Graduate Student, Computer Science
New Mexico State University
🔗 LinkedIn •  Email