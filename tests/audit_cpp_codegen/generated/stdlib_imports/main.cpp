#include <iostream>
#include <vector>
#include <SomeLib.hpp>
#include <Arduino.h>
#include "header/stdlib_imports.hpp"


int main() {
    std::vector<int> values = {1, 2, 3};
    auto touched = touch_stream(std::cout);
    auto base = some_lib_bonus(values.size());
    return arduino_pin(base) + touched;
}

