#pragma once

#include <iostream>
#include <string>

constexpr int OUTPUT = 1;
constexpr int HIGH = 1;
constexpr int LOW = 0;

inline void pinMode(int, int) {}
inline void digitalWrite(int, int) {}
inline void delay(int) {}

struct SerialStub {
    void begin(int) {}

    template <typename T>
    void println(const T &value) {
        std::cout << value << '\n';
    }
};

inline SerialStub Serial;

