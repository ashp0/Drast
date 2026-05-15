#include "runtime/drast_runtime.hpp"

struct Node;

struct Node {
    Node() = default;
    Node(int value, std::string label) : value(value), label(std::move(label)) {}
    int value;
    std::string label;
};

void appendBang(std::string& text);
int readNode(Node& node);

void appendBang(std::string& text) {
    text += "!";
}

int readNode(Node& node) {
    return node.value;
}

int main(int argc, char **argv) {
    drast::setArgs(argc, argv);
    std::string label = "root";
    appendBang(label);
    std::string* pointer = &label;
    std::string copied = *pointer;
    Node stack = Node(7, copied);
    std::shared_ptr<Node> heap = std::make_shared<Node>(9, copied);
    std::shared_ptr<Node> alias = heap;
    return readNode(stack) + alias->value;
}

