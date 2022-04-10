<div align="center">

![Drast Logo](resources/Logo.png?)

# Drast Programming Language

![issues](https://img.shields.io/github/issues/Malvion/drast?style=flat-square)
![license](https://img.shields.io/github/license/Malvion/drast?style=flat-square)

A general-purpose language written in cpp20, that is meant to be a more modernized version of C, while still maintaining
its simplicity.

[Docs](docs/docs.md) | [TODO](TODO.md)

</div>

> **NOTE:** This is still in its development phase, so expect some bugs and missing features.

## Features

- **Super Simple and Easy to learn syntax** - Similar to C++
- **Semi-Object Oriented** - Structs have functions and initializers
- **Templates** - Different Syntax than C++
- More are yet to come...

## Example

```c
import io

int :: main(int argc, string[] argv) {
    int variable = 40
    print("Hello World!", variable, "\n")
    
    return 0
}
```

## Building

Drast is built in a similar manner for Windows, macOS, and Linux.

Before you start building Drast, you must install these dependencies listed below:

- Windows: Install CMake and Visual Studio
- macOS: `brew install cmake`
- Linux: `sudo apt install git cmake`

Run the following command after the required dependencies are install:

```batch
git clone --recursive https://github.com/Malvion/drast
cd drast
mkdir build
cd build
cmake ..
```

If all goes well, Drast should be built and ready to run! If you have encountered any errors, please report them in
the 'Issues' tab!

*NOTE:* For Windows, the build directory will have a Visual Studio Solution file. Double click this file, then build and
run.

## Social

- [Discord Server](https://discord.gg/ZbmHzNmzPH)
