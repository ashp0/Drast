#include <string>
#include <utility>
#include <vector>

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

int main() {
    std::vector<int> numbers = {10, 20, 30};
    Box<std::string> box = Box<std::string>("payload");
    std::string text = identity(box.value);
    int first = firstValue(std::move(numbers));
    return first + text.size();
}

