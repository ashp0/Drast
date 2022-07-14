# Structs

## Declaring structs

Structs in the Drast programming language have similarity to the Swift programming language, not in syntax but in
design.
Structs are able to have initializers, destructors, string among many other functions.

```
struct Car:
    company: CarBrand
    year: string
    ownerName: string
    
    fn .init(company: CarBrand, year: string, ownerName: string):
        // NOTE: Assigning variables will happen automatically in newer versions of Drast
        self.company = company
        self.year = year
        self.ownerName = ownerName
```

## Built-in virtual functions

While structs don't have subclassing or overriding, they do have built-in virtual functions.

### Calling

Built-in virtual functions are called using the `->` operator.

```
myStruct := MyStruct('John', 'Doe', 25)
myStruct->str() // Virtual function
myStruct.str() // Non-virtual function
```

### List of functions

List of all the virtual functions.

```
/* The initializer
 * Arguments are allowed
 * No return
 */
fn .init()

/* The deinitializer
 * Arguments are not allowed
 * No return
 */
fn .deinit()

/* When the struct will be printed
 * Arguments are not allowed
 * Has return 
 */
fn .str() -> string
```

## FAQ

### Where to place variables

Variables are ***highly*** recommended to be placed on the top of the struct. Variables shouldn't be declared in between
the structure.

### Name case

It is a good exercise to use CamelCase when declaring structs normally. However, there are sometimes exceptions. 