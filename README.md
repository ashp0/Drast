<div align="center">

# Drast Programming Language

![issues](https://img.shields.io/github/issues/Malvion/drast?style=flat-square)
![license](https://img.shields.io/github/license/Malvion/drast?style=flat-square)

A general-purpose language written in cpp20, that is meant to be a more modernized version of C, while still maintaining its simplicity.

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

When trying to build drast, the process to build drast is similar across Windows, MacOS and Linux.

In order to build drast, you first download and install a few dependencies/apps either from your terminal or offline:
    
    For Windows, download and install CMake and Visual Studio
    For MacOS, do `brew install cmake`
    For Linux, do `sudo apt install git cmake`
    
After downloading and installing the required dependencies/apps, in your terminal, do:
```batch
git clone --recursive https://github.com/Malvion/drast
cd drast
mkdir build
cd build
cmake ..
```
    
If all goes well, drast should be built and ready to run! If there is any error, please report them in the 'Issues' tab!

NOTE: For Windows, inside the build directory you will find a Visual Studio Code file. Double click this file, then build and run.

## Social

- [Our Discord Server](https://discord.gg/ZbmHzNmzPH)
