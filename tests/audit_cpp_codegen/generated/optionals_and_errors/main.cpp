#include "runtime/drast_runtime.hpp"

std::optional<int> findEven(int value);
int forceEven(int value);

std::optional<int> findEven(int value) {
    if (value % 2 == 0) {
        return value;
    } else {
        return std::nullopt;
    }
}

int forceEven(int value) {
    std::optional<int> candidate = findEven(value);
    try {
        return candidate.value();
    } catch (const std::exception& error) {
        return 0;
    }
}

int main(int argc, char **argv) {
    drast::setArgs(argc, argv);
    int first = forceEven(8);
    int second = forceEven(9);
    return first + second;
}

