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
    - [ ] Cast ( Still need to fix a few things )
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
- [ ] Semantic Analyzer
    - [x] Duplicate Variable and Functions
    - [x] Check if expression is valid
    - [x] Check assignments
    - [x] Return statements
    - [ ] Struct initializers
    - [ ] Alias Types
- [ ] Optimizations? ( Might not do this stage since LLVM does this )
- [ ] Code Generation
- [ ] Improvements to the CLI
- [ ] Standard Library
- [ ] Self Compile

## Example

```swift
import io

int :: main(int argc, string[] argv) {
    print("Hello World!")
    
    return 0
}
```

## Documentation

The Documentation is in the `docs` folder

_NOTE: This programming language is a WIP, things may change_