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
    - [x] Cast
    - [x] Switch, break, case, default
    - [x] For, While, Continue
    - [x] Union
    - [x] Do Catch Try
    - [x] matches keyword
    - [x] Import
    - [x] Functions
    - [x] Struct
    - [x] Enums
- [ ] Improvements
  - [ ] Lexer: Problem where is there are block comments, the lexer will loop
  - [ ] Parser: Problem where using brackets in expressions didn't work
- [ ] Syntax Checker
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