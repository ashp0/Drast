# Structs

They are the same thing as C structs, except they can have functions declarations

Must be defined as UpperCamelCase

`MyStruct`,
`Person`,
`Student`,
`University`,

```swift
struct Hello {
    int test
    string test
    void *myTest?
    
    private void :: printHelloWorld() {
        print("Hello World")
    }
}
```

```
int myFunction(Hello hello) {
    hello.printHelloWorld()
}
```