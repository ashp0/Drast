# Drast

The Drast Programming Language that is meant to be a modernized and better version of C, while still maintaining its
simplicity

## TODO

- [x] Lexing
- [x] Parsing
    - [x] Variables
    - [x] If, Else, Else If
    - [x] Return
    - [x] ASM
    - [x] Switch, break, case, default
    - [x] For, While, Continue
    - [x] Union
    - [x] Do Catch Try
    - [x] matches keyword
    - [x] Import
    - [x] Functions
    - [x] Struct
    - [x] Enums
- [x] Improvements
    - [x] Lexer: Problem where is there are block comments, the lexer will loop
    - [x] Parser: Improvements with Optional Semi-colons
    - [x] Parser: Problem where using brackets in expressions didn't work
    - [x] Improve the if statement AST
    - [ ] Add support for booleans
    - [ ] Add support for casting
    - [ ] Check if function has a return type
- [ ] Semantic Analyzer
    - [x] Duplicate Variable and Functions
    - [ ] Duplicate Struct Initializers
    - [x] Check Expressions
    - [x] Struct initializers
    - [x] Check if self.xxx is an actual member in the struct
    - [ ] Variable Assignments
    - [x] Function Calls
    - [x] Return Statement
    - [ ] If Statements
    - [ ] Alias Types
- [ ] Optimizations? ( Might not do this stage since LLVM does this )
- [ ] Code Generation
- [ ] Improvements to the CLI
- [ ] Standard Library
- [ ] Self Compile

## Example

```c
import io

int :: main(int argc, string[] argv) {
    print("Hello World!")
    
    return 0
}
```

## Documentation

The Documentation is in the `docs` folder

_NOTE: This programming language is a WIP, things may change_
