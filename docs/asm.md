# ASM

ASM follows NASM Syntax,

NOTE: You will not need to add comma's

```swift
func main(argc: Int, argv: String[]) -> Int {
    asm {
    "loop:"
        "cli"
        "hlt"
        "jmp .loop"
    }
    
    return 0
}
```