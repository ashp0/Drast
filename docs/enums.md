# Enums

Similar to Swift's Syntax

```
enum Cars {
    case toyota = 0,
    case honda = 5,
    case jeep, // 6
    case dodge, // 7
}

func testFunction(myEnum: Cars) {
    print(myEnum)
}

func main(argc: Int, argv: String[]) -> Int {
    testFunction(.toyota)
    
    return 0
}
```