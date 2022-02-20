# Try

Similar to Swift Syntax

```swift
do {
    var myVariable = try parseJSON("[{"first" : 50, "second", 100}]")
} catch {
    print("Error Parsing JSON)
}
```

```swift
// TODO: Add Support for error
func myFunction(value: Int) throws -> Int {
    if (value < 100) {
        throw 50
    }
    
    return 100
}
```