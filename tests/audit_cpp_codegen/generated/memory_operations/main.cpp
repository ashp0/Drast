#include <memory>
#include <string>
#include <utility>

struct Node;

struct Node {
    Node() = default;
    Node(int value, std::string label) : value(value), label(std::move(label)) {}
    int value;
    std::string label;
};

void appendBang(std::string& text);
int readNode(const Node& node);

void appendBang(std::string& text) {
    text += "!";
}

int readNode(const Node& node) {
    return node.value;
}

int main() {
    std::string label = "root";
    appendBang(label);
    const std::string& pointer = label;
    std::string copied = pointer;
    Node stack = Node(7, copied);
    std::unique_ptr<Node> heap = std::make_unique<Node>(9, std::move(copied));
    const Node& alias = *heap;
    return readNode(std::move(stack)) + alias.value;
}

