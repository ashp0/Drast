# Structs

Similar to swifts syntax

```swift
struct Student {
    var name: String
    var lastName: String
    var age: Int
    var grade: Int
}

func main(argc: Int, argv: String[]) -> Int {
    var harry = Student(name: "Harry", lastName: "Potter", 21, 11)
    var patrick = Student(name: "Patrick", lastName: "Star", 7, 1)
    
    print(patrick)
    
    return 0
}
```