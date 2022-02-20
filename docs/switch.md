# Switch

Switch statements are similar to C's, except it doesn't go to the next case

To tell it to go to the next case, you have to use the `continue` keyword

```c
switch (op) {
    case '+':
        return .addition
    case '-':
        return .subtraction
    case '*':
        return .multiplication
    case '/':
        return .division
    default:
        print("Unknown Operator: {op}")
}
```