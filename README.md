# Drast

The Drast Programming Language that is meant to be a modernized and better version of C, while still maintaining its
simplicity

## TODO

- [x] Lexing
- [x] Parsing
    - [ ] Variables
    - [ ] If, Else, Else If
    - [ ] Return
    - [ ] ASM
    - [ ] Switch, break, case, default
    - [ ] For, While, Continue
    - [ ] Union
    - [ ] Do Catch Try
    - [ ] matches keyword
    - [ ] Import
    - [ ] Functions
    - [ ] Struct
    - [ ] Enums
- [ ] Semantic Analyzer
    - [ ] Duplicate Variable and Functions
    - [ ] Duplicate Struct Initializers
    - [ ] Check Expressions
    - [ ] Struct initializers
    - [ ] Check if self.xxx is an actual member in the struct
    - [ ] Variable Assignments
    - [ ] Function Calls
    - [ ] Return Statement
    - [ ] If Statements
    - [ ] Alias Types
    - [ ] Struct Variable Members
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