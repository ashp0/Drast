# Automatic Type Conversion - Concepts

- Automatic Type Conversion is when a function parameter is expecting a certain type, but your variable has a different type, so ATC convert it automatically. 
- ATC will work only if the variable's type can be converted into the type that function is expecting

## Example

```Swift
let number = getInput()
add(5, number)
// The string variable, number, is automatically converted into a float value. 
```

## Language Designer Notes
I have always found that converting to other types were very annoying, 
and that it wasted line space vertically, so I have decided to 
implement a Automatic Type Conversion feature!
