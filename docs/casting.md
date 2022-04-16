# Casting

If there is a force cast, then there might be an error

```C
float a = 10.5
int b = cast!(a, int) // force cast
int? b = cast?(a, int) // optional cast
```