# Keywords

## matches

**Set a variable's value corresponding to a value. Similar to switch statements**

> `matches (<variable>) {[<value>:]}`

- `variable` The variable that will be compared
- `value` If the variable is equal to this value, the a value will be returned

```C
result = matches(operator) {
        '+': first_number + second_number
        '-': first_number - second_number
        '*': first_number * second_number
        '/': first_number / second_number
        '%': first_number % second_number
        _ {
            throw("Invalid operator")
        }
}
```

**Note:**

- If you need to use more than one command, then you have to use the `{` and `}` brackets.
- `_` is the default operator.
- If you don't use the `_` operator, it will check if it's an optional value, if it is; the variable will be set to nil.
  If not, the compiler will throw an error. The compiler will also send a warning

**Use Cases:**

- Instead of using switch statements to just set the value of a variable, you can use matches. ( Switch statements are
  meant for when you need to check if a value is equal to a certain value. And then do more operations, this is only 1
  operation, which is setting the value of a variable. )

## asm

**Inline Assembler**

> `asm {["<instructions>"]}`

- `instructions` Assembly Instructions

```C
asm {
"loop:"
    "cli"
    "hlt"
    "jmp loop"
}
```

**Note:**

- Comma's are optional

**Use Cases:**

- If you're building an operating system, you can use inline assembler to build your kernel or run special nasm
  instructions.

## enum

**Enumeration**

> `<attributes> enum [<enum_cases>]`

- `attributes` The attributes to the enum, support: `private`

```C
private enum Test {
    case Test = 10,
    case Test,
    case Test,
}
```

**Note:**

- Comma's are required,
- `case` token is also required
- You can set the value of the enum using the `=` operator

**Use Cases:**

- Selecting Different Data Type

## struct & union

**Structures and Unions**

Structs and Unions support functions and variable declarations

> `<attributes> (struct/union) [<member>]`

- `attributes` The attributes to the enum, support: `private`

```C
struct Hello {
    int test
    string test
    void *myTest?

    private void :: print_hello_world() {
        print("Hello World")
    }
}

private union Hello2 {
    int test1
    string[] test2
}
```

**Note:**

- Variables and Functions are supported

**Use Cases:**

- Creating Objects

## C

**C Keywords that are used in drast**

- Switch Statements
- Do, While, For
- If, Else, Else If
- Break, Continue, Return
- Typealias = Typedefs
- Variables and constants ( Except instead of const, you have to use let )
- Import = #include, for libraries, you don't use `<`, `>` tokens, you just type the library name `import io`
- Optional Variables, you can use `?` at the declaration of a variable to make it optional. Also, at function return
  types
- Do, Catch, Try (TODO)
- `myVariable -> int`, for casting