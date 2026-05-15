#include "runtime/drast_runtime.hpp"

template <typename T>
struct Box;

template <typename T>
struct Box {
    Box() = default;
    Box(T value) : value(std::move(value)) {}
    T value;
};


template <typename T>
T identity(const T& value) {
    return value;
}

template <typename T>
T firstValue(const std::vector<T>& values) {
    return values[0];
}

int main(int argc, char **argv) {
    drast::setArgs(argc, argv);
    std::vector<int> numbers = {10, 20, 30};
    Box box = Box<std::string>("payload");
    T text = identity(box.value);
    T first = firstValue(numbers);
    return first + text.size();
}

