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

<details>
<summary>Windows</summary>

In order to build drast, you must first download and install git. Then you can create a new directory where you want to install this code and open a command line. Then run: 
    
```batch
git clone --recursive https://github.com/Malvion/drast
cd drast
```
    
After downloading drast, you must then download and install CMake. After downloading and installing CMake, inside the drast directory, create a folder named build. After this, then open the cmake-gui
    
In the cmake-gui:
    
    - Set the source-code directory to be the directory where drast is located
    
    - Set the where the binaries will be built to the build directory you made

Then press configure and then generate. If all goes well, this should create a Visual Studio solution file inside the build directory, and all you need to do is double click the solution file, then build and run the project within Visual Studio.
</details>

## Social

- [Discord Server](https://discord.gg/ZbmHzNmzPH)
