# Enums

Must be defined as UpperCamelCase

`MyEnum`,
`Car`,
`Color`,
`MyEnum2`,

```Swift
enum Foo {
    case A,
    case B = 40,
    case C = 30,
    case D
}
```

```C
int :: myFunction(Foo foo) {
    switch (foo) {
        case A:
            return 1
        case B:
            return 2
        case C:
            return 3
        case D:
            return 4
    }
}
```