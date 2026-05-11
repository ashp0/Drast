# Drast

Drast is a small compiler toolchain project for a custom language with examples, a parser, lexer, and code generation components.

## Overview

- `src/` contains the compiler implementation in C++.
- `runtime/` contains runtime support headers.
- `Examples/` contains sample `.drast` programs you can run or use for testing.
- `build/` contains generated build artifacts.

## Build

To build the project, run:

```sh
make
```

This will compile the compiler tools and generate build artifacts in `build/`.

## Run examples

Use the compiler or run example scripts from the `Examples/` folder. For example:

```sh
# Example: compile or inspect a sample program
cat Examples/hello.drast
```

## Example code snippets

### Hello world

```drast
print("Hello, Drast!")
```

### Arithmetic and output

```drast
let a = 5
let b = 10
print(a + b)
```

### Conditionals

```drast
let value = 7
if value > 5 {
  print("Greater than five")
} else {
  print("Five or less")
}
```

## Where to find more examples

View the full set of sample programs in the repository:

- https://github.com/ashp0/Drast/tree/main/Examples

## Notes

- `src/main.cpp` is the compiler entry point.
- `runtime/drast_runtime.hpp` contains runtime support types and helpers.

If you want, I can also add a quick `Usage` section for the compiler invocation syntax.
