# Drast

Drast is a small self-hosted compiler toolchain project for a custom language with examples, a lexer, parser, AST model, and C++ code generation components.

## Overview

- `src/` contains the compiler implementation in `.drast`.
- `runtime/` contains runtime support headers.
- `Examples/` contains sample `.drast` programs you can run or use for testing.
- `tests/` contains compiler regression fixtures.
- `SYNTAX.md` documents the implemented language syntax.

## Build

To build the project, run:

```sh
xmake
```

This bootstraps the compiler from the prebuilt seed binary, transpiles the Drast source to generated C++, and compiles the primary `transpiler` target.

Useful targets:

```sh
xmake build bootstrap
xmake build self-host
xmake build test
xmake project -k xcode
```

## Run examples

Use the compiler or inspect sample programs from the `Examples/` folder. For example:

```sh
xmake run -w . transpiler Examples/hello.drast
```

## Example code snippets

### Hello world

```drast
print 'Hello, Drast!'
```

### Arithmetic and output

```drast
a = 5
b = 10
print a + b
```

### Conditionals

```drast
value = 7
if value isgt 5
  print 'Greater than five'
else
  print 'Five or less'
```

## Inspiration
* The goal of Drast is to minimize the amount of times the shift-key is pressed when programming. I do not like having to parse a nice block of code, needing to type the right symbol and being slowed down by it. Additionally, I never liked the C++ syntax, full of these unnecessary symbols that add more friction to coding. And not just the C++ syntax, many other languages just add so many redundant symbols, it is absurd!
* This programming language is going to be one I will use for a very long time in the future (although the future of coding is rough), I still would type using it. The long-term goals for this are direct C++ interoperability; so while other co-workers write in the ugly and bloated C++, I can swiftly write in Drast and at the end, transpile it down to the ugly C++ syntax and it is expected that everything will work. 
* For right now, the goal is to get Drast to transpile itself which is almost near. After that, we can simply do a 1:1 refactoring of C++ to Drast code and all the features of Drast_CPP will be automatically included in Drast_Drast.

## Where to find more examples

View the full set of sample programs in the repository:

- https://github.com/ashp0/Drast/tree/main/Examples

## Notes

- `src/main.drast` is the compiler entry point.
- `runtime/drast_runtime.hpp` contains runtime support types and helpers.
