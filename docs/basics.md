# Basics of Drast

Drast is a new programming language that is made to be easy to learn, easy to use and have a simple syntax. This
language is heavily inspired by many famous programming languages.

Note: Some parts of the documentation is copied
from [Swift Documentation](https://docs.swift.org/swift-book/LanguageGuide/TheBasics.html)

## Variables

Variables start with a type (such as the `int` or `string`) and a name (such as `total_number_of_students`
or `hello_world_msg`) and an optional value (such as `50` or `Hello World`).

### Declaring variables

Variables must be declared before they’re used. Variables are declared with the type first, followed by the name and
optional value. Here’s an example of how variables can be used in the Drast programming language:

```C++
int total_number_of_students = 40
string hello_world_msg = "Hello World"
```

## Comments

Comments are declared in the same manner as most programming languages.

### Line comments

Line comments start with a `//` and end with a new line.

```C
// Hello, this is a line comment
```

### Multiline comments

Multiline comments start with a `/*` and end with `*/`

```C
/*
 * Hello, this is a Multiline comment.
 * I can type on this line too!
 */
```

## Semicolon

Unlike many programming languages, Drast doesn't require a semicolon (`;`) at the end of a statement. You can use
semicolon, however, this is only if you want to put 2 or more statements together in a single statement.

```C
int test = 40; string test2 = "Hello, world";
```

## Numbers

Numbers are declared in the way below:

- Integer: `(number)`
- Float: `(number).(number)`
- Hexadecimal: `0x`
- Octal: `0o`
- Binary: `0b`

### Integer numbers

Integer numbers are whole numbers.

```C
int my_integer_number = 302
```

### Float numbers

Float numbers are decimal numbers that are defined similar to integers.

```C
float my_float_number = 35.2
```

### Hexadecimal numbers

Hexadecimal numbers start with `0x` followed by a hexadecimal number.

```C
int hexadecimal_number = 0xFFFFFFFF
```

### Octal numbers

Octal numbers start with `0o` followed by an octal number.

```C
int hexadecimal_number = 0o7313
```

### Binary numbers

Binary numbers start with `0b` followed by a binary number.

```C
int hexadecimal_number = 0b00000001
```

## Typealias

Type aliases define an alternative name for an existing type. You define type aliases with the typealias keyword, the
alternative name, and the original type name.

```Swift
typealias binary = int
```

Once you have defined type aliases, you can use it in your code like this:

```C
binary myBinaryNumber = 0b000101
```

## Boolean

Drast has a Boolean type, labeled as `bool`. Boolean values are referred to as logical, because they can only ever be
true or false. Drast provides two Boolean constant values, true and false:

```C++
bool the_sky_is_blue = true
bool the_grass_is_green = false
```

## Optional

You use optionals in situations where a value may be absent. An optional represents two possibilities: Either there is a
value, and you can unwrap the optional to access that value, or there isn’t a value at all.

Let's say you have a string literal that contains a number `"123"`, and you want to convert it into an integer. The
conversion function will return an optional value, because not every string can be converted into an integer. The
string "123" can be converted into the numeric value 123, but the string `"Hello World"` doesn’t have an obvious numeric
value to convert to.

This is an example of an optional values:

```C
string my_string = "123"
int? my_int = my_string.convertTo("int")
```

### nil

If the conversion between string an integer fails, it will return `nil`. This means that the value is absent.

```C
string my_string = "Hello, World!"
int? my_int = my_string.convertTo("int")

if (my_int == nil) {
    print("Error: 'my_string' cannot be converted to an integer")
}
```

## If statements and force unwrapping

You can use an if statement to find out whether an optional contains a value by comparing the optional against nil. You
perform this comparison with the “equal to” operator (==) or the “not equal to” operator (!=).

```Swift
if (my_int != nil) {
    print(my_int!)
}
```

Since you know the `my_int` has an existing value, you can use `!` to force unwrap the value safely.

## Error Handling

You use error handling to respond to error conditions your program may encounter during execution.

In contrast to optionals, which can use the presence or absence of a value to communicate success or failure of a
function, error handling allows you to determine the underlying cause of failure, and, if necessary, propagate the error
to another part of your program.

When a function encounters an error condition, it throws an error. That function’s caller can then catch the error and
respond appropriately.

Functions which throw errors, must have a `!` at the end of their type signature.

```Swift
enum ErrorFunctionError {
    case TestError
}

int! :: error_function(bool throws_error) {
    if (throws_error) {
        throw ErrorFunctionError.TestError
    }
    
    return 40
}
```

You can now use `do`, `catch` and `try` in your program to handle the error condition

```swift
do {
    int my_int = try error_function(true)
} catch (int err) {
    switch (err) {
        case ErrorFunctionError.TestError:
            print("TestError")
        default:
            print("Unknown error")
    }
}
```